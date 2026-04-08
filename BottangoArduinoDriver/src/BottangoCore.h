#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "BasicCommands.h"
#include "ModulesResponder.h"
#include "AbstractMultiMessageOutgoingSource.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
#include "../BottangoArduinoModules.h"
#include "Time.h"
#include "Outgoing.h"
#include <Arduino.h>

#ifdef USE_ESP32_WIFI
#include <WiFi.h>
#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
#include "PersistentConfigUtil.h"
#endif

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#include "CommandStreamProvider.h"
#endif

#ifdef RELAY_SUPPORTED
#include "IRelayComms.h"
#include "RelayChildPool.h"
#endif

namespace BottangoCore
{
    void bottangoSetup();

    void bottangoLoop();

    void stop(bool doUninitialize);

    void uninitialize();

    void updateReadBuffer(bool secondary);

    bool executeCommand(char *commandString, bool secondary);

    unsigned long getMSTimeOfCommand(char *commandString, bool returnStartTime);

    bool externalCommandIsAllowed(char *commandString, bool secondary);

    bool isOffline();

    extern EffectorPool effectorPool;
    extern AbstractMultiMessageOutgoingSource *activeOutgoingMultimessage;
    extern bool initialized;
    extern bool handshake;
    extern char delimiters[];

    extern char serialCommandBuffer[MAX_COMMAND_LENGTH];
    extern int serialCommandIdx;

    extern unsigned long timeOfLastChar;
    extern bool commandInProgress;
    extern char *splitCommandBuffer[COMMANDS_PARAMS_SIZE];

    void initUSBSerialComms();

    bool rcvAvailable(bool secondary);
    char readNextChar(bool secondary);

#if defined(RELAY_SUPPORTED)
    void initRelayComs();

    extern RelayChildPool *relayPool;
    extern bool isRelayBridge;
    extern bool isRelayPeer;
    extern IRelayComms *relayComs;

    extern char *secondaryPeerCommandBuffer;
    extern int secondaryCommandIdx;
    extern unsigned long secondaryTimeOfLastChar;
    extern unsigned long lastPollTimeAsPeer;
    extern bool secondaryCommandInProgress;
    extern int thisPeerID;
    extern bool hasPeerId;

#ifdef RELAY_LOGGING
    extern unsigned long lastWaitForConnectLog;
#endif
#endif

#ifdef USE_ESP32_WIFI
    void initESP32WifiComs();
    void onWifiConnetionSuccess();
    void onWifiConnectionClosed();
    bool updateWifiConnectionStatus();
#endif

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    extern CommandStreamProvider *commandStreamProvider;
#endif

#ifdef USE_ESP32_WIFI
    extern WiFiClient client;
#endif

} // namespace BottangoCore

#endif // BOTTANGO_CORE_H
