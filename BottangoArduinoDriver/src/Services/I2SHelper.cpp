#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SHelper.h"
#include "Outgoing.h"

namespace I2SHelper
{
    uint8_t buffer[I2S_BUFFER_SIZE];
    uint8_t cacheBuffer[I2S_BUFFER_SIZE];
    uint8_t volumeBuffer[I2S_BUFFER_SIZE];
    size_t cachedBytes = 0;
    volatile bool playing = false;

    char currentFilePath[MAX_FILE_PATH_SIZE] = {0};
    uint32_t currentByteOffset = 0;
    char pendingFilePath[MAX_FILE_PATH_SIZE] = {0};
    uint32_t pendingByteOffset = 0;
    bool fileOnDeck = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    I2SClass i2s;
    uint32_t rate;
    i2s_data_bit_width_t bitsPerSample;
    i2s_slot_mode_t channelFormat;
#else
    i2s_config_t i2sConfig;
    i2s_pin_config_t pinConfig = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};
#endif
#endif

#ifdef DYNAMIC_VOLUME
    uint16_t lastVolumeRead = 0;
    unsigned long lastVolumeTime = 0;
#endif

    TaskHandle_t i2sTaskHandle = nullptr; // Task handle for i2S task
    volatile bool stopTask = false;

    // request to stop current playback but keep I2S/task alive
    static volatile bool stopPlaybackRequested = false;
    // request to reconfigure I2S clock/format to new header values
    static volatile bool reconfigureRequested = false;
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    void startPlaying(uint32_t newRate, i2s_data_bit_width_t newBitsPerSample, i2s_slot_mode_t newChannelFormat, const char *filePath, uint32_t byteOffset)
#else
    void startPlaying(i2s_config_t newConfig, const char *filePath, uint32_t byteOffset)
#endif
#endif
    {
#ifdef DYNAMIC_VOLUME
        pinMode(VOLUME_PIN, INPUT);
        lastVolumeRead = analogRead(VOLUME_PIN);
        lastVolumeTime = millis();
#endif

#ifdef PIN_ON_AUDIO_PLAY
        digitalWrite(AUDIO_ENABLE_PIN, LOW);
        pinMode(AUDIO_ENABLE_PIN, OUTPUT);
#endif

        bool configChanged = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        // Did the audio format actually change?
        if (rate != newRate ||
            bitsPerSample != newBitsPerSample ||
            channelFormat != newChannelFormat)
        {
            configChanged = true;
        }

        // Store new audio config from WAV header
        I2SHelper::rate = newRate;
        I2SHelper::bitsPerSample = newBitsPerSample;
        I2SHelper::channelFormat = newChannelFormat;
#else
        // Did the audio format actually change?
        if (i2sConfig.sample_rate != newConfig.sample_rate ||
            i2sConfig.bits_per_sample != newConfig.bits_per_sample ||
            i2sConfig.channel_format != newConfig.channel_format)
        {
            configChanged = true;
        }

        // Store new audio config from WAV header
        I2SHelper::i2sConfig = newConfig;
#endif
#endif

        // queue new file for playback
        strncpy(pendingFilePath, filePath, sizeof(pendingFilePath));
        pendingByteOffset = byteOffset;
        fileOnDeck = true;

        // only ask the task to reconfigure if:
        //  - the task is already running, AND
        //  - the format actually changed
        if (i2sTaskHandle != nullptr && configChanged)
        {
            reconfigureRequested = true;
        }

        // ensure the I2S task is running (in case init() wasn't called)
        if (i2sTaskHandle == nullptr)
        {
            // This will start I2S using the *current* config (which we just updated),
            // and the task will immediately begin streaming (zeros until the file kicks in).
            init();
        }
        else if (playing)
        {
            // interrupt current playback so the new file can take over
            stopPlaybackRequested = true;
        }
    }

    void init()
    {
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        // If nothing configured yet, pick a default "silence" format
        if (rate == 0)
        {
            rate = I2S_INIT_SAMPLE_RATE;
            bitsPerSample = I2S_DATA_BIT_WIDTH_16BIT;
            channelFormat = I2S_SLOT_MODE_STEREO;
        }
#else
        // If nothing configured yet, pick a default "silence" format
        if (i2sConfig.sample_rate == 0)
        {
            i2sConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
            i2sConfig.sample_rate = I2S_INIT_SAMPLE_RATE;
            i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
            i2sConfig.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
            i2sConfig.communication_format = I2S_COMM_FORMAT_STAND_I2S;
            i2sConfig.intr_alloc_flags = 0;
            i2sConfig.dma_buf_count = I2S_DMA_BUFF_COUNT;
            i2sConfig.dma_buf_len = I2S_DMA_BUFF_LEN;
        }
#endif
#endif
        executeStart();
    }

    void executeStart()
    {
        // only initialize once
        if (i2sTaskHandle != nullptr)
        {
            return;
        }

        stopTask = false;
        stopPlaybackRequested = false;
        cachedBytes = 0;

#ifdef PIN_ON_AUDIO_PLAY
        digitalWrite(AUDIO_ENABLE_PIN, HIGH);
#endif

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        i2s.setPins(I2S_BCLK, I2S_LRC, I2S_DOUT);
        // use whatever config is currently stored (default or from last startPlaying)
        i2s.begin(I2S_MODE_STD, rate, bitsPerSample, channelFormat);
#else
        // use whatever config is currently stored (default or from last startPlaying)
        i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
        i2s_set_pin(I2S_NUM_0, &pinConfig);
#endif
#endif

        // reset buffers
        memset(buffer, 0, sizeof(buffer));
        memset(cacheBuffer, 0, sizeof(cacheBuffer));
        memset(volumeBuffer, 0, sizeof(volumeBuffer));

        xTaskCreate(
            i2sTask,       // Task function
            "i2SPlay",     // Name of the task
            4096,          // Stack size in words
            NULL,          // Task input parameter
            1,             // Priority of the task
            &i2sTaskHandle // Task handle
        );
    }

    bool isPlaying()
    {
        return playing;
    }

    void stopPlaying()
    {
        // request current playback to stop; task will switch to zeros
        stopPlaybackRequested = true;
    }

#ifdef DYNAMIC_VOLUME
    void updateVolume()
    {
        if (millis() - lastVolumeTime > VOLUME_READ_INTERVAL)
        {
            lastVolumeTime = millis();
            lastVolumeRead = analogRead(VOLUME_PIN);
        }
    }
#endif

    void i2sTask(void *param)
    {
        SDCardUtil::SDFileError err = SDCardUtil::SDFileError::ERR_NONE;
        File audioFile;

        while (!stopTask)
        {
            // handle external request to stop current playback
            if (stopPlaybackRequested)
            {
                stopPlaybackRequested = false;
                playing = false;
                cachedBytes = 0;
                SDCardUtil::closeFile(audioFile);
                audioFile = File();
            }

            // handle pending I2S reconfiguration (e.g., first real audio after silence)
            if (reconfigureRequested)
            {
                reconfigureRequested = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                i2s.end();
                i2s.begin(I2S_MODE_STD, rate, bitsPerSample, channelFormat);
#else
                // map channel_format to mono/stereo for clock
                i2s_channel_t ch = I2S_CHANNEL_STEREO;
                if (i2sConfig.channel_format == I2S_CHANNEL_FMT_ONLY_RIGHT)
                {
                    ch = I2S_CHANNEL_MONO;
                }
                i2s_set_clk(I2S_NUM_0, i2sConfig.sample_rate, i2sConfig.bits_per_sample, ch);
#endif
#endif
            }

            // handle a new file queued for playback
            if (!playing && fileOnDeck)
            {
                SDCardUtil::closeFile(audioFile);
                audioFile = File();

                // openFile already does its own lockCard/unlockCard.
                audioFile = SDCardUtil::openFile(pendingFilePath, err);
                if (err == SDCardUtil::SDFileError::ERR_NONE && audioFile)
                {
                    SDCardUtil::lockCard();
                    audioFile.seek(pendingByteOffset);
                    SDCardUtil::unlockCard();

                    strncpy(currentFilePath, pendingFilePath, sizeof(currentFilePath));
                    currentByteOffset = pendingByteOffset;
                    playing = true;
                }
                else
                {
                    playing = false;
                }

                fileOnDeck = false;
            }

            size_t bytesRead = 0;

            // normal audio playback path
            if (playing && audioFile)
            {
                SDCardUtil::lockCard();
                bool hasMore = (audioFile.available() || cachedBytes > 0);
                SDCardUtil::unlockCard();

                if (!hasMore)
                {
                    // end of file
                    playing = false;
                    SDCardUtil::closeFile(audioFile);
                    audioFile = File();
                }
                else
                {
                    // restore cached to main buffer
                    if (cachedBytes > 0)
                    {
                        bytesRead = cachedBytes;
                        memcpy(buffer, cacheBuffer, cachedBytes); // Copy cached data to the primary buffer
                        cachedBytes = 0;                          // Clear cache
                    }

                    SDCardUtil::lockCard();
                    bool canRead = audioFile.available() && bytesRead < I2S_BUFFER_SIZE;
                    SDCardUtil::unlockCard();

                    // fill remaining buffer with read from SD
                    if (canRead)
                    {
                        SDCardUtil::lockCard();
                        size_t additionalBytes = audioFile.read(volumeBuffer, I2S_BUFFER_SIZE - bytesRead);
                        SDCardUtil::unlockCard();

#ifdef DYNAMIC_VOLUME
                        float min = VOLUME_MIN;
                        float max = VOLUME_MAX;
                        float volume = min + (max - min) * ((float)lastVolumeRead / 4095.0);
#else
                        float volume = 0.1f; // Volume scale factor (0.0 to 1.0)
#endif

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                        switch (bitsPerSample)
                        {
                        case I2S_DATA_BIT_WIDTH_8BIT:
#else
                        switch (i2sConfig.bits_per_sample)
                        {
                        case I2S_BITS_PER_SAMPLE_8BIT:
#endif
#endif
                        {
                            uint8_t *sampleBuffer8 = volumeBuffer;
                            for (size_t i = 0; i < additionalBytes; i++)
                            {
                                sampleBuffer8[i] = static_cast<uint8_t>(sampleBuffer8[i] * volume);
                            }
                            break;
                        }
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                        case I2S_DATA_BIT_WIDTH_16BIT:
#else
                        case I2S_BITS_PER_SAMPLE_16BIT:
#endif
#endif
                        {
                            int16_t *sampleBuffer16 = reinterpret_cast<int16_t *>(volumeBuffer);
                            for (size_t i = 0; i < additionalBytes / 2; i++)
                            {
                                sampleBuffer16[i] = static_cast<int16_t>(sampleBuffer16[i] * volume);
                            }
                            break;
                        }
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                        case I2S_DATA_BIT_WIDTH_24BIT:
#else
                        case I2S_BITS_PER_SAMPLE_24BIT:
#endif
#endif
                        {
                            uint8_t *byteBuffer24 = volumeBuffer;
                            for (size_t i = 0; i < additionalBytes / 3; i++)
                            {
                                size_t base = i * 3;
                                int32_t sample = (int32_t)((byteBuffer24[base + 0]) |
                                                           (byteBuffer24[base + 1] << 8) |
                                                           (byteBuffer24[base + 2] << 16));
                                if (sample & 0x00800000)
                                {
                                    sample |= 0xFF000000;
                                }
                                sample = static_cast<int32_t>(sample * volume);
                                byteBuffer24[base + 0] = sample & 0xFF;
                                byteBuffer24[base + 1] = (sample >> 8) & 0xFF;
                                byteBuffer24[base + 2] = (sample >> 16) & 0xFF;
                            }
                            break;
                        }
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                        case I2S_DATA_BIT_WIDTH_32BIT:
#else
                        case I2S_BITS_PER_SAMPLE_32BIT:
#endif
#endif
                        {
                            int32_t *sampleBuffer32 = reinterpret_cast<int32_t *>(volumeBuffer);
                            for (size_t i = 0; i < additionalBytes / 4; i++)
                            {
                                sampleBuffer32[i] = static_cast<int32_t>(sampleBuffer32[i] * volume);
                            }
                            break;
                        }
                        }

                        memcpy(buffer + bytesRead, volumeBuffer, additionalBytes);
                        bytesRead += additionalBytes; // Update bytesRead with the newly read data
                    }
                }
            }

            // if no audio data was produced this iteration, stream zeros
            if (bytesRead == 0)
            {
                memset(buffer, 0, I2S_BUFFER_SIZE);
                bytesRead = I2S_BUFFER_SIZE;
            }

            if (bytesRead > 0)
            {
                size_t bytesWritten = 0;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
                bytesWritten = i2s.write(buffer, bytesRead);
                if (bytesWritten < bytesRead)
                {
#else
                esp_err_t result = i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, I2S_WRITE_TIMEOUT);
                if (result != ESP_OK || bytesWritten < bytesRead)
                {
#endif
#endif
                    size_t unwrittenBytes = bytesRead - bytesWritten;
                    memcpy(cacheBuffer, buffer + bytesWritten, unwrittenBytes);
                    cachedBytes = unwrittenBytes;
                }
            }
        }

        // full teardown requested
        playing = false;
#ifdef PIN_ON_AUDIO_PLAY
        digitalWrite(AUDIO_ENABLE_PIN, LOW);
#endif
        SDCardUtil::closeFile(audioFile);

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        i2s.end();
#else
        i2s_driver_uninstall(I2S_NUM_0);
#endif
#endif

        stopTask = false;
        i2sTaskHandle = nullptr;

        vTaskDelete(NULL);
    }

    void cleanup()
    {
        // optional: fully stop I2S subsystem
        if (i2sTaskHandle != nullptr)
        {
            stopTask = true;
        }
    }
}
#endif