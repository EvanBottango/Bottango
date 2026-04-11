#include "../BottangoArduinoModules.h"
#if defined(AUDIO_SD_I2S) && defined(ENABLE_ESP_OTA_UPDATE)

#ifndef AUDIO_BINARY_UTIL_H
#define AUDIO_BINARY_UTIL_H

#include <Arduino.h>
#include "DataSource/SDCardUtil.h"

namespace AudioBinaryUtil
{
	void beginAudioBinary(char* audioIdentifier, bool isHash);
	void recvAudioBinaryData(const char* hexData);
	void processAudioBinaryData(uint8_t* buffer, size_t dataLength);
	void finishAudioBinary(const char* expectedChecksumStr);

	extern bool audioBinaryInProgress;
	extern File writeFile;
}

#endif
#endif