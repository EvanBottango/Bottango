#include "BottangoCore.h"

#include "../BottangoArduinoModules.h"
#include "Module Handling/ModuleMaster.h"
#include "DataSource/DataSource.h"
#include "System/SystemStatus.h"
#include "I2SAudioEffector.h"
#include "Modules/RelayComs/Relay.h"

/*#if defined(RELAY_SUPPORTED) && defined(RELAY_COMS_ESPNOW)
#include "ESPNOWUtil.h"
#endif*/

namespace BottangoCore
{
	EffectorPool effectorPool = EffectorPool();
	AbstractMultiMessageOutgoingSource* activeOutgoingMultimessage = nullptr;

	ModuleMaster mMaster = ModuleMaster();

	char delimiters[] = ",";
	//char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];

/*#ifdef RELAY_SUPPORTED
	RelayChildPool* relayPool = nullptr;
	bool isRelayBridge = false;
	bool isRelayPeer = false;
	char* secondaryPeerCommandBuffer = nullptr;
	int secondaryCommandIdx = 0;
	unsigned long secondaryTimeOfLastChar = 0;*/
	//unsigned long lastHeartbeatTime = 0;
	//bool secondaryCommandInProgress = false;
#ifdef RELAY_LOGGING
	unsigned long lastWaitForConnectLog = 0;
#endif

#if defined(RELAY_SUPPORTED)
	unsigned long lastPollTimeAsPeer;
#endif // RELAY_SUPPORTED
//#endif

//#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
//	CommandStreamProvider* commandStreamProvider = nullptr;
//#endif

#if !defined(RELAY_SUPPORTED) && defined(USE_ESP32_WIFI)
	WiFiClient client;
	bool serverConnected = false;
	unsigned long lastNetworkCheckTime = 0;
	const unsigned long NETWORK_CHECK_INTERVAL_MS = 15000; // recheck wifi connection every 15 seconds
	const unsigned long SERVER_CHECK_INTERVAL_MS = 3000;   // recheck server every 3 seconds
#endif

	void bottangoSetup()
	{
		PersistentConfigUtil::setUseExportedCommandStream(true);

		// Set the initial connection status. This can be overwritten by a module during initModules()
		SystemStatus::systemStatus.PowerStatus = SystemStatus::ePowerStatus::Ok;
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
		SystemStatus::systemStatus.Signal = SystemStatus::eSignal::SDError;

		mMaster.setupModules();
		mMaster.initModules();


#ifdef NAMED_BOARD_STARTUP
		NamedBoardStartup::runNamedBoardStartup();
#endif

		// take pins low on launch
#ifdef PIN_LOW_LAUNCH
		for (int i = 0; i < PIN_REMAP_LENGTH; i++)
		{
			pinMode(onboardPins[i], INPUT_PULLDOWN);
		}
#endif

		// init comms types
#if defined(USE_ESP32_WIFI)
		initESP32WifiComs();
#endif

		// init status lights
#ifdef ENABLE_STATUS_LIGHTS
		SystemStatus::systemStatus.Signal = SystemStatus::eSignal::Off;
		SystemStatus::systemStatus.UserLED = SystemStatus::eUserLED::Off;
		//StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
		//StatusLights::setDesiredColor(USER_STATUS_LIGHT, CRGB::Black);
#endif

// enter exported animation if required
//#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
		// ToDo: implement dynamic source switch <- das müsste eigentlich drin sein
//        if (PersistentConfigUtil::getUseExportedCommandStream())
//#endif
//#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
//        {
			// Note: Moved to mMaster.initModules();
//#ifdef ENABLE_STATUS_LIGHTS
			//SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Export_Playback;
			//StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_CONNECTION_EXPORT_PLAYBACK);
			//StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_PULSE);
//#endif
			/*commandStreamProvider = new CommandStreamProvider();
			SystemStatus::systemStatus.initialized = true;
			Callbacks::onThisControllerStarted();
			commandStreamProvider->runSetup();*/
			//       }
		   //#endif

		// Setup is done
		SystemStatus::systemStatus.initialized = true;
		Callbacks::onThisControllerStarted();
	}

#ifdef USE_ESP32_WIFI

	void initESP32WifiComs()
	{
#ifdef ESP32WIFI_LOGGING
		Serial.begin(BAUD_RATE);
#endif
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

	void onWifiConnetionSuccess()
	{
		lastNetworkCheckTime = 0;
		serverConnected = false;
	}

	void onWifiConnectionClosed()
	{
		BasicCommands::stop(nullptr);
		// restart the board
		ESP.restart();
	}

	bool updateWifiConnectionStatus()
	{
		// attempt wifi connection?
		if (WiFi.status() != WL_CONNECTED)
		{
			// ready to check?
			if (lastNetworkCheckTime == 0 || Time::getCurrentTimeInMs() - lastNetworkCheckTime >= NETWORK_CHECK_INTERVAL_MS)
			{
				WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
				lastNetworkCheckTime = Time::getCurrentTimeInMs();
			}
			// not gonna read/write anything this loop...
			return false;
		}
		// attempt server connection?
		else if (!serverConnected)
		{
			// ready to check?
			if (lastNetworkCheckTime == 0 || Time::getCurrentTimeInMs() - lastNetworkCheckTime >= SERVER_CHECK_INTERVAL_MS)
			{
				lastNetworkCheckTime = Time::getCurrentTimeInMs();
				// connect to the server
				if (client.connect(WIFI_SERVER_IP, WIFI_SERVER_PORT))
				{
					Outgoing::printOutputStringFlash(F("\n\n"));
					Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
					Outgoing::printOutputStringFlash(F("\n\n"));
					serverConnected = true;
				}
				else
				{
					// couldn't connect, we'll try again
					return false;
				}
			}
		}
		// had then lost server connection
		else if (!client.connected())
		{
			BasicCommands::stop(nullptr);
			// restart the board
			ESP.restart();
			return false;
		}

		return true;
	}

#endif

	static void stopPlaybackModule()
	{
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
		mMaster.getModule<AnimationPlaybackControl>(Modules::AnimPlaybackCntrl)->stop();
#endif
	}

	static void hardStop()
	{
		Callbacks::onThisControllerStopped();
		uninitialize();

#ifdef ENABLE_STATUS_LIGHTS
#ifdef RELAY_SUPPORTED
		if (mMaster.getModule<Relay>(Modules::RelayComs)->getRole() == Relay::RelayRole::Peer)
		{
			SystemStatus::eConnectionStatus::No_Connection_Peer;
		}
		else
		{
			SystemStatus::eConnectionStatus::No_Connection_Serial;
		}
#else
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
#endif
#endif
	}

	void request_Stop()
	{
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
		{
			stopPlaybackModule();
		}
		else
		{
			OutgoingSerial::printOutputStringPROGMEM(BasicCommands::STOP_PLAY);
		}
	}

	void request_eStop()
	{
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
		{
			stopPlaybackModule();
			hardStop();
		}
		else
		{
			OutgoingSerial::printOutputStringPROGMEM(BasicCommands::ESTOP);
		}
	}

	void stop()
	{
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
		{
			stopPlaybackModule();
		}
		hardStop();
	}

	/*bool splitIntoBuffer(char* stringToSplit, byte& paramsCount)
	{
#ifdef RELAY_SUPPORTED
		// Special case if first token is relay pass through if is relay bridge
		// first token is sR, second is peer id, third token is all of the command, fourth is the hash
		if (isRelayBridge
			&& strncmp(stringToSplit, BasicCommands::PASS_TO_RELAY, strlen(BasicCommands::PASS_TO_RELAY)) == 0
			&& stringToSplit[strlen(BasicCommands::PASS_TO_RELAY)] == ',')
		{
			// Find first comma (after sR)
			char* firstComma = strchr(stringToSplit, ',');
			if (!firstComma)
			{
				return false;
			}

			// Find second comma (after ID)
			char* secondComma = strchr(firstComma + 1, ',');
			if (!secondComma)
			{
				return false;
			}

			// Find last comma (for hash split)
			char* lastComma = strrchr(secondComma + 1, ',');
			if (!lastComma || lastComma <= secondComma)
			{
				return false;
			}

			// Set splitCommandBuffer[0] = sR
			size_t len0 = firstComma - stringToSplit;
			static char sRbuffer[8]; // enough for sR\0
			strncpy(sRbuffer, stringToSplit, len0);
			sRbuffer[len0] = '\0';
			splitCommandBuffer[0] = sRbuffer;

			// Set splitCommandBuffer[1] = ID
			size_t len1 = secondComma - (firstComma + 1);
			static char idBuffer[8]; // enough for id + \0
			strncpy(idBuffer, firstComma + 1, len1);
			idBuffer[len1] = '\0';
			splitCommandBuffer[1] = idBuffer;

			if (isOffline())
			{
				// Set splitCommandBuffer[2] = everything after 2nd comma
				// except trim last char if it's a \n newline
				static char offlinePayloadBuffer[MAX_COMMAND_LENGTH];
				strcpy(offlinePayloadBuffer, secondComma + 1);

				size_t payloadLen = strlen(offlinePayloadBuffer);
				if (payloadLen > 0 && offlinePayloadBuffer[payloadLen - 1] == '\n')
				{
					offlinePayloadBuffer[payloadLen - 1] = '\0';
				}

				splitCommandBuffer[2] = offlinePayloadBuffer;

				// Set splitCommandBuffer[3] = to an empty string, since there's no hash in an offline command
				static char emptyString[] = "";
				splitCommandBuffer[3] = emptyString;
			}
			else
			{
				// Set splitCommandBuffer[2] = everything between 2nd and last comma
				size_t len2 = lastComma - (secondComma + 1);
				static char payloadBuffer[MAX_COMMAND_LENGTH];
				strncpy(payloadBuffer, secondComma + 1, len2);
				payloadBuffer[len2] = '\0';
				splitCommandBuffer[2] = payloadBuffer;

				// Set splitCommandBuffer[3] = hash (after last comma)
				splitCommandBuffer[3] = lastComma + 1;
			}

			paramsCount = 4;
			return true;
		}
#endif*/
		// Regular tokenization
		/*byte idxResult = 0;
		char *token = strtok(stringToSplit, delimiters);

		while (token != NULL)
		{
			if (idxResult >= COMMANDS_PARAMS_SIZE) // Check buffer capacity
			{
				Error::reportError_TooManyParams(idxResult);
				return false;
			}
			splitCommandBuffer[idxResult++] = token;
			token = strtok(NULL, delimiters);
		}

		paramsCount = idxResult;*/
		//return true;
	//}

	bool checkHash(char* cmdString)
	{
		if (cmdString[0] == '\0')
		{
			return 0;
		}

		char c = cmdString[0];
		int idx = 0;

		// Scan forward to end of string
		while (c != '\0')
		{
			c = cmdString[idx++];
		}
		// Scan backward to find start of hash (don't hash the hash)
		idx -= 1; // One for 'h', one for ','

		while (idx > 0)
		{
			if (cmdString[idx] == ',' && cmdString[idx + 1] == 'h')
			{
				break;
			}
			idx--;
		}

		int hashStartIdx = idx + 2;

		idx -= 1; // For ','

		int hsh = 0;
		while (idx >= 0)
		{
			c = cmdString[idx--];
			hsh += c;
		}

		int ati = atoi(cmdString + hashStartIdx);

		return ati == hsh;
	}

	void uninitialize()
	{
		BottangoCore::effectorPool.deregisterAll();
		while (Serial.available() > 0)
		{
			char t = Serial.read();
		}
		SystemStatus::systemStatus.initialized = false;
		//initialized = false;

		DataSource* dataSource = mMaster.getModule<DataSource>(Modules::DataSource_Serial);
		dataSource->resetBuffer();
		//commandInProgress = false;
		//serialCommandIdx = 0;
		//serialCommandBuffer[serialCommandIdx] = '\0';
		//timeOfLastChar = 0;
	}

	void bottangoLoop()
	{
		Callbacks::onEarlyLoop();
		mMaster.executePhase(Phase::Input);
		mMaster.executePhase(Phase::Communication);
		mMaster.executePhase(Phase::Logic);
		mMaster.executePhase(Phase::Output);

		//updateReadBuffer(false); // standard read

/*#ifdef RELAY_SUPPORTED
		if (isRelayPeer)
		{
			updateReadBuffer(true); // secondary read when peer also
		}
#endif*/

		if (SystemStatus::systemStatus.initialized)
		{

			/*#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
						if (isOffline())
			#endif
			#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
						{
							if (commandStreamProvider != nullptr)
							{
								commandStreamProvider->updateOnLoop();
							}
						}
			#endif*/

			// update effector pool
			effectorPool.updateAllDriveTargets();

			// update relay pool
#ifdef RELAY_SUPPORTED
			if (mMaster.getModule<Relay>(Modules::RelayComs)->isBridge())
			{
				mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->update();
			}
			else if (mMaster.getModule<Relay>(Modules::RelayComs)->isPeer() && millis() - lastPollTimeAsPeer > RELAY_POLL_TIMEOUT_AS_PEER)
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("Lost Bridge!"));
				OutgoingSerial::printLine();
				//Outgoing::endToggleOnSecondaryOutgoing();
				BasicCommands::reboot(false);
			}
#endif
		}

		// update outgoing multimessage sender if any
		if (activeOutgoingMultimessage != nullptr)
		{
			activeOutgoingMultimessage->updateMultiMessage();
			if (activeOutgoingMultimessage->multiMessageisComplete())
			{
				activeOutgoingMultimessage->cleanUpMultiMessage();
				activeOutgoingMultimessage = nullptr;
			}
		}

		// update prefs reset monitoring
#ifdef RESET_PREFS_SUPPORTED
		PersistentConfigUtil::update();
#endif

#ifdef NAMED_BOARD_LOOP
		NamedBoardLoop::runNamedBoardLoop();
#endif

		Callbacks::onLateLoop();

#if defined(RELAY_SUPPORTED) && defined(RELAY_LOGGING)
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{			
			if (mMaster.getModule<Relay>(Modules::RelayComs)->isPeer() && !SystemStatus::systemStatus.initialized && Time::getCurrentTimeInMs() - lastWaitForConnectLog >= 1000)
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("Waiting for bridge...\n"));
				//Outgoing::endToggleOnSecondaryOutgoing();
				lastWaitForConnectLog = Time::getCurrentTimeInMs();
			}
		}
#endif // RELAY_SUPPORTED && RELAY_LOGGING
	}

	/*bool isOffline()
	{
		bool offline = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
		if (commandStreamProvider != nullptr)
		{
			offline = true;
		}
#else
		offline = true;
#endif
#endif
		return offline;
	}*/

	void updateReadBuffer(bool secondary)
	{
		/*
		#if defined(USE_ESP32_WIFI)
				if !(updateWifiConnectionStatus())
				{
					return;
				}
		#endif

		#ifdef RELAY_SUPPORTED
				if (isRelayPeer && secondary)
				{
					Outgoing::setSecondaryPeerOutgoing(true);
				}
		#endif

				while (rcvAvailable(secondary))
				{
					char read = readNextChar(secondary);

		#ifdef RELAY_SUPPORTED

					if (secondary)
					{
						secondaryCommandInProgress = true;
						secondaryTimeOfLastChar = millis();
					}
					else
		#endif
					{
						commandInProgress = true;
						timeOfLastChar = millis();
					}

					if (read == '\n')
					{

						bool hashPasses;
		#ifdef RELAY_SUPPORTED
						if (secondary)
						{
							hashPasses = checkHash(secondaryPeerCommandBuffer);
						}
						else
		#endif
						{
							hashPasses = checkHash(serialCommandBuffer);
						}

		#ifdef RELAY_SUPPORTED
						if (secondary)
						{
							secondaryCommandInProgress = false;
							secondaryTimeOfLastChar = 0;
						}
						else
		#endif
						{
							commandInProgress = false;
							timeOfLastChar = 0;
						}

						if (hashPasses)
						{
		#ifdef RELAY_SUPPORTED
							// primary incoming on relay peer
							if (isRelayPeer && !secondary)
							{
		#ifdef RELAY_LOGGING
		#ifdef TOGGLE_DEBUG
								if (PersistentConfigUtil::debugEnabled())
		#endif
								{
									Outgoing::toggleOnSecondaryOutgoing();
									Outgoing::printOutputStringFlash(F("peer rcv msg: "));
									Outgoing::printOutputStringMem(serialCommandBuffer);
									Outgoing::printLine();
									Outgoing::endToggleOnSecondaryOutgoing();
								}
		#endif
							}
		#endif

							bool sendReady = true;
		#ifdef RELAY_SUPPORTED
							if (secondary)
							{
								if (externalCommandIsAllowed(secondaryPeerCommandBuffer, secondary))
								{
									sendReady = executeCommand(secondaryPeerCommandBuffer, true);
								}
							}
							else
		#endif
							{
								if (externalCommandIsAllowed(serialCommandBuffer, secondary))
								{
									sendReady = executeCommand(serialCommandBuffer, false);
								}
							}
							if (sendReady)
							{
								Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
							}
						}
						else
						{
							Outgoing::printOutputStringPROGMEM(BasicCommands::HASH_FAIL);
						}

		#ifdef RELAY_SUPPORTED
						if (secondary)
						{
							secondaryCommandIdx = 0;
							secondaryPeerCommandBuffer[secondaryCommandIdx] = '\0';
						}
						else
		#endif
						{
							serialCommandIdx = 0;
							serialCommandBuffer[serialCommandIdx] = '\0';
						}
					}
					else if (read == '\0')
					{
					}
					else
					{
		#ifdef RELAY_SUPPORTED
						if (secondary)
						{
							if (secondaryCommandIdx <= MAX_COMMAND_LENGTH - 2)
							{
								secondaryPeerCommandBuffer[secondaryCommandIdx++] = read;
								secondaryPeerCommandBuffer[secondaryCommandIdx] = '\0';
							}
							else
							{
								Error::reportError_CmdTooLong(secondaryCommandIdx);
								secondaryCommandIdx = 0;
								secondaryPeerCommandBuffer[secondaryCommandIdx] = '\0';

								secondaryCommandInProgress = false;
								secondaryTimeOfLastChar = 0;

								Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
							}
						}
						else
		#endif
						{
							if (serialCommandIdx <= MAX_COMMAND_LENGTH - 2)
							{
								serialCommandBuffer[serialCommandIdx++] = read;
								serialCommandBuffer[serialCommandIdx] = '\0';
							}
							else
							{
								Error::reportError_CmdTooLong(serialCommandIdx);
								serialCommandIdx = 0;
								serialCommandBuffer[serialCommandIdx] = '\0';

								commandInProgress = false;
								timeOfLastChar = 0;

								Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
							}
						}
					}
				}

		#ifdef RELAY_SUPPORTED
				if (secondary)
				{
					if (secondaryCommandInProgress && millis() - secondaryTimeOfLastChar >= READ_TIMEOUT)
					{
						Outgoing::printOutputStringPROGMEM(BasicCommands::TIMEOUT);

						secondaryCommandIdx = 0;
						secondaryPeerCommandBuffer[secondaryCommandIdx] = '\0';

						secondaryCommandInProgress = false;
						secondaryTimeOfLastChar = 0;
					}
				}
				else
		#endif
				{
					if (commandInProgress && millis() - timeOfLastChar >= READ_TIMEOUT)
					{
						Outgoing::printOutputStringPROGMEM(BasicCommands::TIMEOUT);

						serialCommandIdx = 0;
						serialCommandBuffer[serialCommandIdx] = '\0';

						commandInProgress = false;
						timeOfLastChar = 0;
					}
				}

		#ifdef RELAY_SUPPORTED
				if (isRelayPeer && secondary)
				{
					Outgoing::setSecondaryPeerOutgoing(false);
				}
		#endif*/
	}

	bool rcvAvailable(bool secondary)
	{
		/*#ifdef RELAY_SUPPORTED
			if (isRelayPeer)
			{
				if (secondary)
				{
					return Serial.available() > 0;
				}
				else
				{
					return ESPNowUtil::peerRecvAvailable();
				}
			}
			else
			{
				return Serial.available() > 0;
			}

	#elif defined(USE_USB_SERIAL)
			return Serial.available() > 0;*/

#if defined(USE_ESP32_WIFI)
		return client.available() > 0;
#endif
		return '\0';
	}

	char readNextChar(bool secondary)
	{
		/*#ifdef RELAY_SUPPORTED
			if (isRelayPeer)
			{
				if (secondary)
				{
					return Serial.read();
				}
				else
				{
					return ESPNowUtil::peerReadNextChar();
				}
			}
			else
			{
				return Serial.read();
			}
	#elif defined(USE_USB_SERIAL)
			return Serial.read();*/

#if defined(USE_ESP32_WIFI)
		return client.read();
#endif
		return '\0';
	}

} // namespace BottangoCore
