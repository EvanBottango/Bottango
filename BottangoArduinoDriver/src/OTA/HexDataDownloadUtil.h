#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(ENABLE_ESP_OTA_UPDATE)

#include <Arduino.h>

namespace HexDataDownloadUtil
{
	typedef void (*DataReceivedCallback)(uint8_t* buffer, size_t length);

	void beginData(DataReceivedCallback callback);
	void recvData(const char* hexData);
	void finishData();

	extern DataReceivedCallback dataCallback;
	extern bool downloadInProgress;
	extern uint32_t checksum;

}

#endif