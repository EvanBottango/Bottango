#include "../BottangoArduinoModules.h"
#if defined(ENABLE_ESP_OTA_UPDATE)

#ifndef OTA_UPDATE_UTIL_H
#define OTA_UPDATE_UTIL_H

#include <Arduino.h>
#include <Update.h>

namespace OTAUpdateUtil
{
    void beginOTA();
    void recvOTAData(const char *hexData);
    void finishOTA(const char *expectedChecksumStr);

    extern bool otaInProgress;
    extern uint32_t checksum;
}

#endif
#endif