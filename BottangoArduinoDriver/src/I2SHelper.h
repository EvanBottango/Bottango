#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#ifndef I2SHelper_h
#define I2SHelper_h

#include "SDCardUtil.h"

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include "ESP_I2S.h"
#else
#include "driver/i2s.h"
#endif
#endif

#include "../BottangoArduinoConfig.h"

namespace I2SHelper
{
#define I2S_WRITE_TIMEOUT (20 / portTICK_PERIOD_MS) // Short timeout for non-blocking operation
#define I2S_BUFFER_SIZE 512

    extern uint8_t buffer[];
    extern uint8_t cacheBuffer[];
    extern uint8_t volumeBuffer[];
    extern size_t cachedBytes;
    extern bool playing;
    extern File audioFile;
    extern File audioFileOnDeck;
    extern bool fileOnDeck;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    void startPlaying(uint32_t rate, i2s_data_bit_width_t bitsPerSample, i2s_slot_mode_t channelFormat, File audioFile);
    extern I2SClass i2s;
    extern uint32_t rate;
    extern i2s_data_bit_width_t bitsPerSample;
    extern i2s_slot_mode_t channelFormat;
#else
    void startPlaying(i2s_config_t i2sConfig, File audioFile);
    extern i2s_config_t i2sConfig;
    extern i2s_pin_config_t pinConfig;
#endif
#endif

    extern TaskHandle_t i2sTaskHandle;
    extern volatile bool stopTask;

    void executeStart();
    bool isPlaying();
    void cleanup();
    void stopPlaying();

    void i2sTask(void *param);
}

#endif
#endif