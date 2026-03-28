#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "BasicCommands.h"
#include "ModulesResponder.h"
#include "AbstractMultiMessageOutgoingSource.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
#include "../BottangoArduinoModules.h"
#include "Module Handling/ModuleMaster.h"
#include "System/Time.h"
#include "Modules/OutgoingSerial.h"
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

//#ifdef RELAY_SUPPORTED
//#include "Modules/RelayComs/RelayChildPool.h"
//#endif

namespace BottangoCore
{
    void bottangoSetup();
    void bottangoLoop();

	void request_Stop();
	void request_eStop();
    void stop();

    void uninitialize();

    void updateReadBuffer(bool secondary);

    //bool isOffline();

    extern EffectorPool effectorPool;
    extern AbstractMultiMessageOutgoingSource *activeOutgoingMultimessage;
	extern ModuleMaster mMaster;

	extern char delimiters[];

    bool rcvAvailable(bool secondary);
    char readNextChar(bool secondary);

#if defined(RELAY_SUPPORTED)
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
