// I2SAudioEffector.cpp
#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#include "TriggerCurve.h"
#include "System/Time.h"
#include "../BottangoArduinoConfig.h"
#include "Modules/Outgoing.h"
#include "BottangoCore.h"
#include "Modules/Audio/I2SAudEventStatusResponder.h"
#include "Module Handling/ModuleMaster.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
I2SAudioEffector::I2SAudioEffector(/*char** args*/char* identifier, char* hash) : AbstractEffector(0, 1)
{
    strcpy(_myIdentifier, identifier);
    Callbacks::onEffectorRegistered(this);

    char effectorIdentifier[9];
	// ToDo: Why do we copy the identifier into "myIdentifier" and then immediately read it back out into "effectorIdentifier"? Why not just use "myIdentifier" directly?
    getIdentifier(effectorIdentifier, 9);

	_audioModule = BottangoCore::mMaster.getModule<I2SAudioModule>(Modules::AudioI2S);
	_audioModule->checkAudioSource(effectorIdentifier, hash);
}

void I2SAudioEffector::updateOnLoop()
{
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
        _shouldFire = true;
        targetCurve->consumed = true;
        _offsetMS = targetCurve->offset;
    }
}

void I2SAudioEffector::driveOnLoop()
{
    if (_shouldFire)
    {
        if (!_audioModule->isAudioSourceOK())
        {
            _shouldFire = false;
            AbstractEffector::driveOnLoop();
            AbstractEffector::callbackOnDriveComplete(0, false);
            return;
        }

        char effectorIdentifier[9];
        effectorIdentifier[0] = '\0';
        getIdentifier(effectorIdentifier, 9);

		_audioModule->play(effectorIdentifier, _offsetMS);

        _shouldFire = false;
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
    strcpy(outArray, _myIdentifier);
}

void I2SAudioEffector::clearCurves()
{
    AbstractEffector::clearCurves();

    // todo this is wrong
    if (_audioModule->isPlaying())
    {
		_audioModule->stop();
    }
}
#endif