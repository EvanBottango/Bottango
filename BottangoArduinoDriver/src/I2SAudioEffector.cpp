// I2SAudioEffector.cpp
#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"
#include "BottangoCore.h"
#include "I2SAudEventStatusResponder.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
I2SAudioEffector::I2SAudioEffector(char *identifier, char *hash) : AbstractEffector(0, 1)
{
    strcpy(myIdentifier, identifier);
    Callbacks::onEffectorRegistered(this);

    SDCardUtil::SDFileError fileError;

    // Open Hash File
    char effectorIdentifier[9];
    char hashBuffer[FILE_HASH_LENGTH + 1];
    getIdentifier(effectorIdentifier, 9);
    char filePathBuffer[MAX_FILE_PATH_SIZE];
    SDCardUtil::getAudioHashFilePath(effectorIdentifier, filePathBuffer);

    File hashFile = SDCardUtil::openFile(filePathBuffer, fileError);

    int responderCode = 0;
    // got a hash file
    if (fileError == SDCardUtil::SDFileError::ERR_NONE)
    {
        SDCardUtil::lockCard();
        size_t bytesRead = hashFile.readBytes(hashBuffer, FILE_HASH_LENGTH);
        SDCardUtil::unlockCard();

        hashBuffer[bytesRead] = '\0';

        if (strcmp(hashBuffer, hash) == 0)
        {
            responderCode = I2S_AUDIO_STATUS_READY;
        }
        else
        {
            responderCode = I2S_AUDIO_STATUS_NO_HASH_MATCH_ON_CARD;
        }
    }
    // no hash file
    else
    {
        if (fileError == SDCardUtil::SDFileError::ERR_NO_CARD)
        {
            responderCode = I2S_AUDIO_STATUS_NO_SD_CARD;
        }
        else if (fileError == SDCardUtil::ERR_FILE_NOT_FOUND || fileError == SDCardUtil::ERR_IO || !hashFile)
        {
            responderCode = I2S_AUDIO_STATUS_NO_HASH_ON_CARD;
        }
    }
    SDCardUtil::closeFile(hashFile);

    bool headerOk = false;

    // if we didn't get a bad card error
    // open check and init audio file itself from header
    if (fileError != SDCardUtil::SDFileError::ERR_NO_CARD)
    {
        // open audio file
        filePathBuffer[0] = '\0';
        SDCardUtil::getAudioFilePath(effectorIdentifier, filePathBuffer);
        File audioFile = SDCardUtil::openFile(filePathBuffer, fileError);
        // got audio file
        if (fileError == SDCardUtil::ERR_NONE)
        {
            headerOk = true;

            // Read number of channels (position 22 in WAV header)
            SDCardUtil::lockCard();
            uint16_t numChannels;
            audioFile.seek(22);
            size_t channelsRead = audioFile.read((uint8_t *)&numChannels, sizeof(numChannels));
            if (channelsRead != sizeof(numChannels))
            {
                headerOk = false;
            }
            else
            {
                // Configure I2S based on the number of channels
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                if (numChannels == 1)
                {
                    channel_format = I2S_SLOT_MODE_MONO; // Mono configuration
                }
                else if (numChannels == 2)
                {
                    channel_format = I2S_SLOT_MODE_STEREO; // Stereo configuration
                }
                else
                {
                    headerOk = false;
                }
#else
                if (numChannels == 1)
                {
                    channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT; // Mono configuration
                }
                else if (numChannels == 2)
                {
                    channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT; // Stereo configuration
                }
                else
                {
                    headerOk = false;
                }
#endif
#endif
            }

            // Read WAV header to get the sample rate
            audioFile.seek(24); // Offset to sample rate field
            size_t rateRead = audioFile.read((uint8_t *)&sampleRate, sizeof(sampleRate));
            if (rateRead != sizeof(sampleRate))
            {
                headerOk = false;
            }

            audioFile.seek(34); // Offset to bits per sample field
            size_t bitsRead = audioFile.read((uint8_t *)&bitsPerSample, sizeof(bitsPerSample));
            if (bitsRead != sizeof(bitsPerSample))
            {
                headerOk = false;
            }

            SDCardUtil::unlockCard();

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            // Map bits per sample to I2S bits per sample type
            switch (bitsPerSample)
            {
            case 8:
                i2s_bits_per_sample = I2S_DATA_BIT_WIDTH_8BIT;
                break;
            case 16:
                i2s_bits_per_sample = I2S_DATA_BIT_WIDTH_16BIT;
                break;
            case 24:
                i2s_bits_per_sample = I2S_DATA_BIT_WIDTH_24BIT;
                break;
            case 32:
                i2s_bits_per_sample = I2S_DATA_BIT_WIDTH_32BIT;
                break;
            default:
                headerOk = false;
                break;
            }
#else
            // Map bits per sample to I2S bits per sample type
            switch (bitsPerSample)
            {
            case 8:
                i2s_bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT;
                break;
            case 16:
                i2s_bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
                break;
            case 24:
                i2s_bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT;
                break;
            case 32:
                i2s_bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
                break;
            default:
                headerOk = false;
                break;
            }
#endif
#endif

            if (!headerOk)
            {
                Outgoing::printOutputStringFlash(F("I2S header parse failed for effector: "));
                Outgoing::printOutputStringMem(effectorIdentifier);
                Outgoing::printLine();
            }
            else
            {
                headerParsed = true;
            }
        }
        // couldn't open audio file
        else
        {
            if (fileError == SDCardUtil::SDFileError::ERR_NO_CARD)
            {
                responderCode = I2S_AUDIO_STATUS_NO_SD_CARD;
            }
            else if (fileError == SDCardUtil::ERR_FILE_NOT_FOUND || fileError == SDCardUtil::ERR_IO || !hashFile)
            {
                responderCode = I2S_AUDIO_STATUS_NO_FILE_ON_CARD;
            }
        }
        SDCardUtil::closeFile(audioFile);
    }

    // report status of sd card audio file via a multi message responder
    if (BottangoCore::activeOutgoingMultimessage != nullptr)
    {
        // shouldn't have an active...
        BottangoCore::activeOutgoingMultimessage->cleanUpMultiMessage();
        BottangoCore::activeOutgoingMultimessage = nullptr;
    }
    BottangoCore::activeOutgoingMultimessage = new I2SAudEventStatusResponder(responderCode);
#ifdef RELAY_SUPPORTED
    if (Outgoing::secondaryPeerOutgoing)
    {
        BottangoCore::activeOutgoingMultimessage->setSecondary();
    }
#endif
    BottangoCore::activeOutgoingMultimessage->initializeMultiMessage();

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
        if (!headerParsed)
        {
            shouldFire = false;
            AbstractEffector::driveOnLoop();
            AbstractEffector::callbackOnDriveComplete(0, false);
            return;
        }

        char path[MAX_FILE_PATH_SIZE];
        path[0] = '\0';
        char effectorIdentifier[9];
        effectorIdentifier[0] = '\0';
        getIdentifier(effectorIdentifier, 9);
        SDCardUtil::getAudioFilePath(effectorIdentifier, path);
        SDCardUtil::SDFileError fileError;

        int totalOffset = 44; // Assuming 44-byte standard WAV header
        if (offsetMS > 0)
        {
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            byte numChannels = (channel_format == I2S_SLOT_MODE_STEREO) ? 2 : 1;
#else
            byte numChannels = (channel_format == I2S_CHANNEL_FMT_RIGHT_LEFT) ? 2 : 1;
#endif
#endif

            uint32_t bytesPerSample = (bitsPerSample / 8) * numChannels;
            uint32_t samplesToProgress = (sampleRate * offsetMS) / 1000;
            uint32_t byteOffset = samplesToProgress * bytesPerSample;

            // Skip the WAV header (typically the first 44 bytes) and offset
            totalOffset += byteOffset;
        }

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        I2SHelper::startPlaying(sampleRate, i2s_bits_per_sample, channel_format, path, totalOffset);
#else
        // generate config based on header read at init
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = sampleRate,
            .bits_per_sample = i2s_bits_per_sample,
            .channel_format = channel_format,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = 0,
            .dma_buf_count = 16,
            .dma_buf_len = 256};

        // call to i2S helper to start
        I2SHelper::startPlaying(i2s_config, path, totalOffset);
#endif
#endif

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
    if (I2SHelper::isPlaying())
    {
        I2SHelper::stopPlaying();
    }
}
#endif
