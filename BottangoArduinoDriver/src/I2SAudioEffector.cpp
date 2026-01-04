// I2SAudioEffector.cpp
#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"
#include "BottangoCore.h"
#include "Module Handling/ModuleMaster.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
I2SAudioEffector::I2SAudioEffector(char** args/*char* identifier, char* hash, IAudioPlayback* interface*/) : AbstractEffector(0, 1)
{
	// Register ourself to the effector pool
	BottangoCore::effectorPool.addEffector(this);

	// Parse arguments
	char* identifier = args[1];
	char* hash = args[2];

    strcpy(myIdentifier, identifier);
    Callbacks::onEffectorRegistered(this);

	AudioInterface = static_cast<IAudioPlayback*>(InterfaceRegistry::get(Modules::AudioI2S));;

    char effectorIdentifier[9];
	// ToDo: Why do we copy the identifier into "myIdentifier" and then immediately read it back out into "effectorIdentifier"? Why not just use "myIdentifier" directly?
    getIdentifier(effectorIdentifier, 9);
	AudioInterface->checkAudioSource(effectorIdentifier, hash);

#ifdef RELAY_SUPPORTED
    if (Outgoing::secondaryPeerOutgoing)
    {
        BottangoCore::activeOutgoingMultimessage->setSecondary();
    }
#endif

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    if (BottangoCore::isOffline() && BottangoCore::commandStreamProvider != nullptr && !(responderCode == I2S_AUDIO_STATUS_READY || responderCode == I2S_AUDIO_STATUS_NO_HASH_MATCH_ON_CARD))
    {
        BottangoCore::commandStreamProvider->setInvalidState();
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
        {
            Outgoing::printOutputStringFlash(F("Exported Anim, Audio File SD Error: "));
            Outgoing::printOutputStringMem(identifier);
            Outgoing::printOutputStringFlash(F(" code: "));
            Outgoing::printOutputStringMem(responderCode);
            Outgoing::printLine();
        }

#endif
    }
#endif
}

void I2SAudioEffector::updateOnLoop()
{
	// ToDo: Das kann alles hier bleiben, da es den Effector direkt betrifft und keine Audio-Hardware ansteuert
    unsigned long currentTime = Time::getCurrentTimeInMs();
    TriggerCurve *targetCurve = NULL;

    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        TriggerCurve *curve = (TriggerCurve *)curves[i];
        if (curve == NULL)
        {
            continue;
        }

        if (curve->startTimeInMs <= currentTime)
        {
            if (targetCurve == NULL || curve->startTimeInMs > targetCurve->startTimeInMs)
            {
                targetCurve = curve;
            }
        }
    }

    // If no curves were in progress, go to the final known state
    if (targetCurve != NULL && targetCurve->consumed == false)
    {
        shouldFire = true;
        targetCurve->consumed = true;
        offsetMS = targetCurve->offset;
    }
}

void I2SAudioEffector::driveOnLoop()
{
    if (shouldFire)
    {
        if (!AudioInterface->isAudioSourceOK())
        {
            shouldFire = false;
            AbstractEffector::driveOnLoop();
            AbstractEffector::callbackOnDriveComplete(0, false);
            return;
        }

        char effectorIdentifier[9];
        effectorIdentifier[0] = '\0';
        getIdentifier(effectorIdentifier, 9);

		AudioInterface->play(effectorIdentifier, offsetMS);

        shouldFire = false;
        AbstractEffector::driveOnLoop();
        AbstractEffector::callbackOnDriveComplete(1, true);
    }
    else
    {
        AbstractEffector::driveOnLoop();
        AbstractEffector::callbackOnDriveComplete(0, false);
    }
}

void I2SAudioEffector::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void I2SAudioEffector::clearCurves()
{
    AbstractEffector::clearCurves();

    // todo this is wrong
    if (AudioInterface->isPlaying())
    {
		AudioInterface->stop();
    }
}
#endif