#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#include "Log.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
I2SAudioEffector::I2SAudioEffector(char *identifier, byte audioID) : AbstractEffector(0, 1)
{
    strcpy(myIdentifier, identifier);
    this->audioID = audioID;
    Callbacks::onEffectorRegistered(this);

    char path[MAX_FILE_PATH_SIZE];
    SDCardUtil::initialize();
    if (!SDCardUtil::sdCardAvailable)
    {
        return;
    }
    SDCardUtil::getAudioFilePath(audioID, path);
    File audioFile = SDCardUtil::openFile(path);
    if (!audioFile)
    {
        Outgoing::printOutputStringFlash(F("Audio File Missing"));
        Outgoing::printLine();
        return;
    }

    // Read number of channels (position 22 in WAV header)
    uint16_t numChannels;
    audioFile.seek(22);
    audioFile.read((uint8_t *)&numChannels, sizeof(numChannels));

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
        // Handle more channels or throw error
        // todo error here
        return;
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
        // Handle more channels or throw error
        // todo error here
        return;
    }
#endif
#endif

    // Read WAV header to get the sample rate
    audioFile.seek(24); // Offset to sample rate field
    audioFile.read((uint8_t *)&sampleRate, sizeof(sampleRate));

    audioFile.seek(34); // Offset to bits per sample field
    audioFile.read((uint8_t *)&bitsPerSample, sizeof(bitsPerSample));

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
        // todo error here
        return;
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
        // todo error here
        return;
    }
#endif
#endif

    SDCardUtil::closeFile(audioFile);
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
        if (!SDCardUtil::sdCardAvailable)
        {
            return;
        }

        char path[MAX_FILE_PATH_SIZE];
        SDCardUtil::getAudioFilePath(audioID, path);
        File audioFile = SDCardUtil::openFile(path);

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
            audioFile.seek(44 + byteOffset); // Assuming 44-byte standard WAV header
        }
        else
        {
            // Skip the WAV header (typically the first 44 bytes)
            audioFile.seek(44); // Assuming 44-byte standard WAV header
        }

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        I2SHelper::startPlaying(sampleRate, i2s_bits_per_sample, channel_format, audioFile);
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
        I2SHelper::startPlaying(i2s_config, audioFile);
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