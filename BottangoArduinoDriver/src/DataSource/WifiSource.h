#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef USE_ESP32_WIFI
#include <WiFi.h>
#include "CharStreamedSource.h"

class WifiSource : public CharStreamedSource
{
public:
	void init() override;
	void onPhase(Phase p) override;
	void readData() override;

	void onWifiConnetionSuccess_Internal();
	void onWifiConnectionClosed_Internal();

private:
	WiFiClient _client;
	bool _serverConnected = false;
	unsigned long _lastNetworkCheckTime = 0;
	const unsigned long NETWORK_CHECK_INTERVAL_MS = 15000; // recheck wifi connection every 15 seconds
	const unsigned long SERVER_CHECK_INTERVAL_MS = 3000;   // recheck server every 3 seconds
};

#endif // USE_ESP32_WIFI