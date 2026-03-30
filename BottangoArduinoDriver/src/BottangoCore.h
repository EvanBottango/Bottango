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

namespace BottangoCore
{
    void bottangoSetup();
    void bottangoLoop();

	void request_Stop();
	void request_eStop();

    /**
	 * @brief Stops the playback - only use when the source is Serial! Otherwise, use the request_Stop() or request_eStop() functions
     */
    void stop();

    void uninitialize();

    void updateReadBuffer(bool secondary);

    //bool isOffline();

    extern EffectorPool effectorPool;
    extern AbstractMultiMessageOutgoingSource *activeOutgoingMultimessage;
	extern ModuleMaster mMaster;

	extern char delimiters[];

    //bool rcvAvailable(bool secondary);
    //char readNextChar(bool secondary);

#if defined(RELAY_SUPPORTED)
	extern unsigned long lastPollTimeAsPeer;
#endif // RELAY_SUPPORTED

#ifdef RELAY_LOGGING
    extern unsigned long lastWaitForConnectLog;
#endif // RELAY_LOGGING

#ifdef USE_ESP32_WIFI
    void initESP32WifiComs();
    void onWifiConnetionSuccess();
    void onWifiConnectionClosed();
    bool updateWifiConnectionStatus();
#endif // USE_ESP32_WIFI


#ifdef USE_ESP32_WIFI
    extern WiFiClient client;
#endif // USE_ESP32_WIFI

} // namespace BottangoCore

#endif // BOTTANGO_CORE_H