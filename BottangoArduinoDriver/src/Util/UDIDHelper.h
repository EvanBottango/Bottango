#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED) || defined(REPORT_UID)

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Services/PersistentConfigUtil.h"

#ifdef RELAY_SUPPORTED
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include "esp_mac.h" // 3.x
#else
#include "esp_system.h" // 2.x
#endif
#endif
namespace UDIDHelper
{

#ifdef REPORT_UID
	void convertUIDToCStr(const uint8_t uid[UID_LENGTH], char str[UID_CSTR_SIZE]);
#endif

#ifdef RELAY_SUPPORTED
	void getThisDeviceMacAddress(uint8_t mac[6]);

	bool convertCStrToMAC(const char* str, uint8_t mac[6]);
	void convertMACToCStr(const uint8_t mac[6], char* str);
#endif

}

#endif