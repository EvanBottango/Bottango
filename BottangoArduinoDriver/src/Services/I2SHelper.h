#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef AUDIO_SD_I2S


#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include <ESP_I2S.h>
#else
#include <driver/i2s.h>
#endif
#endif

#include "../../BottangoArduinoConfig.h"
#include "../DataSource/SDCardUtil.h"

namespace I2SHelper
{

	extern uint8_t buffer[];
	extern uint8_t cacheBuffer[];
	extern uint8_t volumeBuffer[];
	extern size_t cachedBytes;
	extern volatile bool playing;

	extern char currentFilePath[MAX_FILE_PATH_SIZE];
	extern uint32_t currentByteOffset;
	extern char pendingFilePath[MAX_FILE_PATH_SIZE];
	extern uint32_t pendingByteOffset;
	extern bool fileOnDeck;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	void startPlaying(uint32_t rate, i2s_data_bit_width_t bitsPerSample, i2s_slot_mode_t channelFormat, const char* filePath, uint32_t byteOffset);
	extern I2SClass i2s;
	extern uint32_t rate;
	extern i2s_data_bit_width_t bitsPerSample;
	extern i2s_slot_mode_t channelFormat;
#else
	void startPlaying(i2s_config_t i2sConfig, const char* filePath, uint32_t byteOffset);
	extern i2s_config_t i2sConfig;
	extern i2s_pin_config_t pinConfig;
#endif
#endif

#ifdef DYNAMIC_VOLUME
	void updateVolume();
#endif

	extern TaskHandle_t i2sTaskHandle;
	extern volatile bool stopTask;

	void init();
	void executeStart();
	bool isPlaying();
	void cleanup();
	void stopPlaying();

	void i2sTask(void* param);
}

#endif
