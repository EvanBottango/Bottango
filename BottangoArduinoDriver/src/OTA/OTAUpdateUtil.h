#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(ENABLE_ESP_OTA_UPDATE)

#include <Arduino.h>
#include <Update.h>

namespace OTAUpdateUtil
{
	void beginOTA();
	void recvOTAData(const char* hexData);
	void processHexData(uint8_t* buffer, size_t dataLength);
	void finishOTA(const char* expectedChecksumStr);

	extern bool otaInProgress;
}

#endif