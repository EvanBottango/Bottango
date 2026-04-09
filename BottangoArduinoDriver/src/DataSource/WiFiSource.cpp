#include "WifiSource.h"

#ifdef USE_ESP32_WIFI
#include "BottangoCore.h"
#include "Module Handling/ModuleMaster.h"

void onWifiConnetionSuccess()
{
	BottangoCore::mMaster.getModule<WifiSource>(Modules::DataSource_Secondary)->onWifiConnetionSuccess_Internal();
}

void onWifiConnectionClosed()
{
	BottangoCore::mMaster.getModule<WifiSource>(Modules::DataSource_Secondary)->onWifiConnectionClosed_Internal();
}


void WifiSource::init()
{
	WifiSource* secondarySource = BottangoCore::mMaster.registerModuleInSecondaryDataSlot<WifiSource>();
	BottangoCore::mMaster.getModule<CommandDecoder>(Modules::Decoder)->setSecondaryDataSource(secondarySource);

	// begin async callback to check for connection state
	WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
		{
			switch (event)
			{

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
			case IP_EVENT_STA_GOT_IP:
				onWifiConnetionSuccess();
				break;
			case WIFI_EVENT_STA_DISCONNECTED:
				onWifiConnectionClosed();
				break;
#else
			case SYSTEM_EVENT_STA_GOT_IP:
				onWifiConnetionSuccess();
				break;
			case SYSTEM_EVENT_STA_DISCONNECTED:
				onWifiConnectionClosed();
				break;
#endif
#endif

			default:
				break;
			}
		});
}

void WifiSource::onPhase(Phase p)
{
	if (p != Phase::Communication)
	{
		return;
	}

	// attempt wifi connection
	if (WiFi.status() != WL_CONNECTED)
	{
		// ready to check?
		if (_lastNetworkCheckTime == 0 || Time::getCurrentTimeInMs() - _lastNetworkCheckTime >= NETWORK_CHECK_INTERVAL_MS)
		{
			WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
			_lastNetworkCheckTime = Time::getCurrentTimeInMs();
		}
		
		// not gonna read/write anything this loop...
		return;
	}
	// attempt server connection
	else if (!_serverConnected)
	{
		// ready to check?
		if (_lastNetworkCheckTime == 0 || Time::getCurrentTimeInMs() - _lastNetworkCheckTime >= SERVER_CHECK_INTERVAL_MS)
		{
			_lastNetworkCheckTime = Time::getCurrentTimeInMs();
			// connect to the server
			if (_client.connect(WIFI_SERVER_IP, WIFI_SERVER_PORT))
			{
				Outgoing::printOutputStringFlash(F("\n\n"));
				Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
				Outgoing::printOutputStringFlash(F("\n\n"));
				_serverConnected = true;
			}
			else
			{
				// couldn't connect, we'll try again
				return;
			}
		}
	}
	// had then lost server connection
	else if (!_client.connected())
	{
		BasicCommands::stop(nullptr);
		// restart the board
		ESP.restart();
		return;
	}

	readData();
}

void WifiSource::readData()
{
	while (_client.available() > 0)
	{
		processData(_client.read());
	}

	checkTimeout();
}

void WifiSource::onWifiConnetionSuccess_Internal()
{
	_lastNetworkCheckTime = 0;
	_serverConnected = false;
}

void WifiSource::onWifiConnectionClosed_Internal()
{
	BottangoCore::request_eStop();
	// restart the board
	ESP.restart();
}
#endif // USE_ESP32_WIFI