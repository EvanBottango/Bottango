#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "BasicCommands.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
#include "../BottangoArduinoModules.h"
#include "Time.h"
#include <Arduino.h>

#ifdef USE_ESP32_WIFI
#include <WiFi.h>
#endif

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#include "DynamicAnimationSwitchMonitor.h"
#endif

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#include "CommandStreamProvider.h"
#endif

#ifdef RELAY_PARENT
#include "RelayChildPool.h"
#endif

namespace BottangoCore
{
    void bottangoSetup();

    void bottangoLoop();

    void stop();

    void executeCommand(char *commandString);

    unsigned long getMSStartTimeOfCommand(char *commandString);

    unsigned long getMSEndTimeOfCommand(char *commandString);

    bool externalCommandIsAllowed(char *commandString);

#ifdef USE_ESP32_WIFI
    void onWifiConnetionSuccess();
    void onWifiConnectionClosed();
#endif

    extern EffectorPool effectorPool;
#ifdef RELAY_PARENT
    extern RelayChildPool relayPool;
#endif
    extern bool initialized;
    extern bool handshake;
    extern char delimiters[];

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    extern CommandStreamProvider *commandStreamProvider;
#endif

#ifdef USE_ESP32_WIFI
    extern WiFiClient client;
#endif

} // namespace BottangoCore

#endif // BOTTANGO_CORE_H
