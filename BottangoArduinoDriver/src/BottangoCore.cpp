#include "BottangoCore.h"

#include "../BottangoArduinoModules.h"

#ifdef RELAY_SUPPORTED
#include "IRelayComms.h"

#ifdef RELAY_COMS_ESPNOW
#include "RelayCommsESPNow.h"
#endif

#ifdef RELAY_COMS_RS485
#include "RelayCommsRS485.h"
#endif

#endif

#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"
#endif

#if defined(AUDIO_SD_I2S) && defined(DYNAMIC_VOLUME)
#include "I2SHelper.h"
#endif

namespace BottangoCore
{
    EffectorPool effectorPool = EffectorPool();
    AbstractMultiMessageOutgoingSource *activeOutgoingMultimessage = nullptr;

#ifdef RELAY_SUPPORTED
    IRelayComms *relayComs = nullptr;
    RelayChildPool *relayPool = nullptr;
    bool isRelayBridge = false;
    bool isRelayPeer = false;
    char *secondaryPeerCommandBuffer = nullptr;
    int secondaryCommandIdx = 0;
    unsigned long secondaryTimeOfLastChar = 0;
    unsigned long lastPollTimeAsPeer = 0;
    bool secondaryCommandInProgress = false;
    int thisPeerID = 0;
    bool hasPeerId = false;
#ifdef RELAY_LOGGING
    unsigned long lastWaitForConnectLog = 0;
#endif
#endif

    char delimiters[] = ",";

    bool initialized = false;
    bool handshake = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    CommandStreamProvider *commandStreamProvider = nullptr;
#endif

#if !defined(RELAY_SUPPORTED) && defined(USE_ESP32_WIFI)
    WiFiClient client;
    bool serverConnected = false;
    unsigned long lastNetworkCheckTime = 0;
    const unsigned long NETWORK_CHECK_INTERVAL_MS = 15000; // recheck wifi connection every 15 seconds
    const unsigned long SERVER_CHECK_INTERVAL_MS = 3000;   // recheck server every 3 seconds
#endif

    char serialCommandBuffer[MAX_COMMAND_LENGTH];
    int serialCommandIdx = 0;

    unsigned long timeOfLastChar = 0;
    bool commandInProgress = false;
    char *splitCommandBuffer[COMMANDS_PARAMS_SIZE];

#ifdef STOP_BUTTON_SUPPORTED
    unsigned long lastStopButtonPressTime = 0;
#endif

    void replaceActiveOutgoingMultimessage(AbstractMultiMessageOutgoingSource *responder)
    {
        if (activeOutgoingMultimessage != nullptr)
        {
            activeOutgoingMultimessage->cleanUpMultiMessage();
            activeOutgoingMultimessage = nullptr;
        }

        activeOutgoingMultimessage = responder;
        if (activeOutgoingMultimessage == nullptr)
        {
            return;
        }

#ifdef RELAY_SUPPORTED
        if (Outgoing::secondaryPeerOutgoing)
        {
            activeOutgoingMultimessage->setSecondary();
        }
#endif

        activeOutgoingMultimessage->initializeMultiMessage();
    }

    void
    bottangoSetup()
    {

#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::initLights();
        StatusLights::setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_RED);
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_RED);
        StatusLights::updateLights();
#endif

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
#if defined(RELAY_SUPPORTED)
        initRelayComs();
#elif defined(USE_USB_SERIAL)
        initUSBSerialComms();
#elif defined(USE_ESP32_WIFI)
        initESP32WifiComs();
#endif

// init status lights
#ifdef ENABLE_STATUS_LIGHTS
#ifdef RELAY_SUPPORTED
        if (isRelayPeer)
        {
            StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_NO_CONNECTION_PEER);
            StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_BLINK);
        }
        else
        {
            StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_NO_CONNECTION_SERIAL);
            StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_BLINK);
        }
#else
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_NO_CONNECTION_SERIAL);
        StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_BLINK);
#endif
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
        StatusLights::setDesiredColor(USER_STATUS_LIGHT, CRGB::Black);
#endif

// init i2s audio
#ifdef AUDIO_SD_I2S
        I2SHelper::init();
#endif

// enter exported animation if required
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
        if (PersistentConfigUtil::getUseExportedCommandStream())
#endif
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
        {
#ifdef ENABLE_STATUS_LIGHTS
            StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_CONNECTION_EXPORT_PLAYBACK);
            StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_PULSE);
#endif
            commandStreamProvider = new CommandStreamProvider();
            initialized = true;
            Callbacks::onThisControllerStarted();
            commandStreamProvider->runSetup();
        }
#endif

#ifdef STOP_BUTTON_SUPPORTED
        pinMode(STOP_BUTTON_PIN, STOP_INPUT_TYPE);
#endif
    }

    void initUSBSerialComms()
    {
        Serial.begin(BAUD_RATE);
#ifdef RELAY_SUPPORTED
        if (isRelayPeer)
        {
            Outgoing::setSecondaryPeerOutgoing(true);
        }
#endif
        Outgoing::printLine();
        Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
        Outgoing::printLine();
#ifdef RELAY_SUPPORTED
        if (isRelayPeer)
        {
            Outgoing::setSecondaryPeerOutgoing(false);
        }
#endif
    }

#if defined(RELAY_SUPPORTED)
    void initRelayComs()
    {
#ifdef RELAY_COMS_ESPNOW
        relayComs = new RelayCommsESPNow();
#elif defined(RELAY_COMS_RS485)
        relayComs = new RelayCommsRS485();
#endif

        int relayState = PersistentConfigUtil::getRelayState();
        if (relayState == VALUE_RELAY_STATE_LIVE_USB)
        {
            initUSBSerialComms();
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled())
#endif
            {
                Outgoing::printOutputStringFlash(F("relay state = usb"));
                Outgoing::printLine();
            }

#endif
        }
        else if (relayState == VALUE_RELAY_STATE_BRIDGE)
        {
            isRelayBridge = true;
            isRelayPeer = false;

            initUSBSerialComms();

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled())
#endif
            {
                Outgoing::printOutputStringFlash(F("relay state = bridge"));
                Outgoing::printLine();
            }

#endif

            relayPool = new RelayChildPool();
            relayComs->initializeAsBridge();

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled())
#endif
            {
                Outgoing::printOutputStringFlash(F("bridge setup complete"));
                Outgoing::printLine();
            }
#endif
        }
        else if (relayState == VALUE_RELAY_STATE_PEER)
        {
            isRelayBridge = false;
            isRelayPeer = true;
            hasPeerId = false;

            secondaryCommandIdx = 0;
            secondaryCommandInProgress = false;
            secondaryTimeOfLastChar = 0;
            if (secondaryPeerCommandBuffer)
            {
                free(secondaryPeerCommandBuffer);
            }
            secondaryPeerCommandBuffer = (char *)malloc(MAX_COMMAND_LENGTH);

            initUSBSerialComms();
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled())
#endif
            {
                Outgoing::toggleOnSecondaryOutgoing();
                Outgoing::printOutputStringFlash(F("relay state = peer. Sending BOOT"));
                Outgoing::printLine();
                Outgoing::endToggleOnSecondaryOutgoing();
                lastWaitForConnectLog = Time::getCurrentTimeInMs();
            }

#endif

#ifdef RELAY_SUPPORTED
            relayComs->initializeAsPeer();
            Outgoing::printLine();
            Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
            Outgoing::printLine();
#endif
        }
    }
#endif

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
            } });
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

    void stop(bool doUninitialize)
    {
        if (isOffline())
        {
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
            if (commandStreamProvider != nullptr)
            {
                if (doUninitialize)
                {
                    commandStreamProvider->forceStopForTeardown();
                }
                else
                {
                    commandStreamProvider->stop();
                }
            }
#endif
        }

        // need to handle a bridge gracefully stopping it's peers?
#ifdef RELAY_SUPPORTED
        if (BottangoCore::isRelayBridge)
        {
            // enque clear cuves on all peers
            relayPool->clearCurvesOnConnectedPeers();

            if (doUninitialize)
            {

                // stop any new animation from playing
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
                if (commandStreamProvider != nullptr)
                {
                    commandStreamProvider->setInvalidState();
                }
#endif

                // send stop out to all peers if not already stopping (and stop all future messages)
                // then wait and let loop come back once the queue is empty
                if (!relayPool->isUninitializing)
                {
                    relayPool->beginPoolTeardown();
                    return;
                }
                // we are uninitializing, but there's still messages to send
                else if (!relayPool->toPeerQueueEmpty())
                {
                    // still wait
                    return;
                }
            }
        }
#endif

        if (doUninitialize)
        {
            Callbacks::onThisControllerStopped();
            uninitialize();

            // stop should only reboot, if in live connection mode.
            // if in
            if (isOffline())
            {
#ifdef ENABLE_STATUS_LIGHTS
                StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_RED);
                StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_BLINK);
#endif
            }
            else
            {
                BasicCommands::reboot(false);
            }
        }
    }

    bool splitIntoBuffer(char *stringToSplit, byte &paramsCount)
    {
#ifdef RELAY_SUPPORTED
        // Special case if first token is relay pass through if is relay bridge
        // first token is sR, second is peer id, third token is all of the command, fourth is the hash
        if (isRelayBridge &&
            strncmp(stringToSplit, BasicCommands::PASS_TO_RELAY, strlen(BasicCommands::PASS_TO_RELAY)) == 0 &&
            stringToSplit[strlen(BasicCommands::PASS_TO_RELAY)] == ',')
        {
            // Find first comma (after sR)
            char *firstComma = strchr(stringToSplit, ',');
            if (!firstComma)
            {
                return false;
            }

            // Find second comma (after ID)
            char *secondComma = strchr(firstComma + 1, ',');
            if (!secondComma)
            {
                return false;
            }

            // Find last comma (for hash split)
            char *lastComma = strrchr(secondComma + 1, ',');
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
#endif
        // Regular tokenization
        byte idxResult = 0;
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

        paramsCount = idxResult;
        return true;
    }

    // parses the serial buffer and turns it into commands
    // param commandString: e.g. "rSP,9,1000,3000"
    bool executeCommand(char *commandString, bool secondary)
    {
        bool sendReady = true;

#ifdef ALLOW_SYNC_COMMANDS
        // before split, check if this is a syncronized command
        // we don't actually want to split a syncronized command, but to parse it's own unique syntax
        if (strncmp_P(commandString, BasicCommands::SYNC_COMMAND, 3) == 0)
        {
            BasicCommands::executeSyncronizedCommands(commandString, secondary);
            return sendReady;
        }
#endif

        byte paramsCount = 0;
        bool splitSuccess = splitIntoBuffer(commandString, paramsCount);
        if (!splitSuccess)
        {
            return sendReady;
        }

        // The command name is the first string in the array, subsequent strings are parameters of that command
        char *commandName = splitCommandBuffer[0];

#ifdef ENABLE_STATUS_LIGHTS
        bool flashCmdRcvLight = true;
#endif
        // to all who may judge a giant list of if / else... I get it.
        // but also, benchamarking proved this to be faster than any other more elegant looking approach
        // so it may be ugly... but it's quick.

        if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0)
        {
            BasicCommands::addCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_INSTANTCURVE) == 0)
        {
            BasicCommands::addInstantCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::STOP) == 0)
        {
            BasicCommands::stop(splitCommandBuffer);
        }
#ifdef RELAY_SUPPORTED
        else if (strcmp_P(commandName, BasicCommands::RELAY_POLL_REQUEST) == 0)
        {
            BasicCommands::requestPoll(splitCommandBuffer);
            sendReady = false;
#ifdef ENABLE_STATUS_LIGHTS
            flashCmdRcvLight = false;
#endif
        }
        else if (strcmp_P(commandName, BasicCommands::PASS_TO_RELAY) == 0)
        {
            BasicCommands::passToRelayController(splitCommandBuffer, paramsCount);
        }
#endif
        else if (strcmp_P(commandName, BasicCommands::TIME_SYNC) == 0)
        {
            BasicCommands::syncTime(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::DEREGISTER_ALL_EFFECTORS) == 0)
        {
            BasicCommands::deregisterAllEffectors(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
        {
            // Ignore duplicate handshake requests after the initial handshake completes.
            if (!secondary && BottangoCore::handshake)
            {
                sendReady = false;
            }
            else
            {
                BasicCommands::sendHandshakeResponse(splitCommandBuffer, secondary);
            }
        }
        else if (strcmp_P(commandName, BasicCommands::MODULES_REQUEST) == 0)
        {
            BasicCommands::startModulesResponse(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::READY_FOR_NEXT_RESPONSE) == 0)
        {
            BasicCommands::continueInProgressMultiMessageResponse(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::CLEAR_ALL_CURVES) == 0)
        {
            BasicCommands::clearAllCurves(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::UPDATE_EFFECTOR_SIGNAL_BOUNDS) == 0)
        {
            BasicCommands::updateEffectorSignalBounds(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0)
        {
            BasicCommands::addOnOffCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0)
        {
            BasicCommands::addTriggerCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::CLEAR_EFFECTOR_CURVES) == 0)
        {
            BasicCommands::clearCurvesForEffector(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
        {
            BasicCommands::addColorCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_INSTANT_COLOR_CURVE) == 0)
        {
            BasicCommands::addInstantColorCurve(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::DEREGISTER_EFFECTOR) == 0)
        {
            BasicCommands::deregisterEffector(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_I2C_SERVO) == 0)
        {
            BasicCommands::registerI2CServo(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_PIN_SERVO) == 0)
        {
            BasicCommands::registerPinServo(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_PIN_STEPPER) == 0)
        {
            BasicCommands::registerPinStepper(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_DIR_STEPPER) == 0)
        {
            BasicCommands::registerDirStepper(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_CURVED_EVENT) == 0)
        {
            BasicCommands::registerCurvedEvent(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_ONOFF_EVENT) == 0)
        {
            BasicCommands::registerOnOffEvent(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_TRIGGER_EVENT) == 0)
        {
            BasicCommands::registerTriggerEvent(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_CUSTOM_MOTOR) == 0)
        {
            BasicCommands::registerCustomMotor(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REGISTER_COLOR_EVENT) == 0)
        {
            BasicCommands::registerColorEvent(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::STEPPER_SYNC) == 0)
        {
            BasicCommands::stepperSync(splitCommandBuffer);
        }
#ifdef AUDIO_SD_I2S
        else if (strcmp_P(commandName, BasicCommands::REGISTER_AUDIO_EVENT) == 0)
        {
            BasicCommands::registerAudioEvent(splitCommandBuffer);
        }
#endif
#ifdef RELAY_SUPPORTED
        else if (strcmp_P(commandName, BasicCommands::REGISTER_RELAY) == 0)
        {
            BasicCommands::registerRelayController(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::DEREGISTER_RELAY) == 0)
        {
            BasicCommands::deregisterRelayController(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::DEREGISTER_ALL_RELAY) == 0)
        {
            BasicCommands::deregisterAllRelayControllers(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::REQUEST_PEER_BOOT) == 0)
        {
            BasicCommands::requestBoot(splitCommandBuffer);
            sendReady = false;
        }
        else if (strcmp_P(commandName, BasicCommands::GET_MAC_ADDRESS) == 0)
        {
            BasicCommands::getMACAddress(splitCommandBuffer);
        }
#endif
#ifdef ENABLE_ESP_OTA_UPDATE
        else if (strcmp_P(commandName, BasicCommands::OTA_UPDATE) == 0)
        {
            BasicCommands::processOTA(splitCommandBuffer);
        }
#endif
#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
        else if (strcmp_P(commandName, BasicCommands::SET_CONFIG) == 0)
        {
            BasicCommands::setConfiguration(splitCommandBuffer);
        }
#endif

#ifdef ENABLE_STATUS_LIGHTS
        if (flashCmdRcvLight)
        {
            StatusLights::pulseSignalLight();
        }
#endif
        return sendReady;
    }

    // note this is destructive to the string
    unsigned long getMSTimeOfCommand(char *commandString, bool returnStartTime)
    {

#ifdef RELAY_SUPPORTED
        // need to trim "sR,Idx" and then recurse back
        if (strncmp_P(commandString, BasicCommands::PASS_TO_RELAY, 2) == 0)
        {
            char *thirdFieldStart = commandString;

            // Find the comma after the 1st field, then again after the 2nd
            for (int skip = 0; skip < 2; ++skip)
            {
                thirdFieldStart = strchr(thirdFieldStart, ','); // strchr returns a pointer to the comma
                ++thirdFieldStart;                              // move one past it, so we land at the start of the next field
            }

            // Now thirdFieldStart points to the first character of the 3rd field.
            // Shift everything from there (including the '\0') down to the front.
            memmove(commandString, thirdFieldStart, strlen(thirdFieldStart) + 1);

            return getMSTimeOfCommand(commandString, returnStartTime);
        }
#endif

#ifdef ALLOW_SYNC_COMMANDS
        // recurse through all sync commands for the earliest start
        if (strncmp_P(commandString, BasicCommands::SYNC_COMMAND, 3) == 0)
        {
            bool firstTimeSet = false;
            unsigned long resultTime = 0;

            char prefixBuffer[CMD_PREFIX_SIZE] = {0};
            char splitCmd[MAX_COMMAND_LENGTH] = {0};
            BasicCommands::beginGetNextSyncCommand(commandString, prefixBuffer);
            while (BasicCommands::getNextSyncCommand(commandString, prefixBuffer, splitCmd))
            {
                unsigned long subCmdTime = getMSTimeOfCommand(splitCmd, returnStartTime);
                if (!firstTimeSet || (returnStartTime && subCmdTime < resultTime) || (!returnStartTime && subCmdTime > resultTime))
                {
                    resultTime = subCmdTime;
                }
            }

            return resultTime;
        }
#endif

        char cmdCopy[MAX_COMMAND_LENGTH] = {0};
        strcpy(cmdCopy, commandString);

        byte paramsCount = 0;
        bool splitSuccess = splitIntoBuffer(commandString, paramsCount);
        if (!splitSuccess)
        {
            return Time::getCurrentTimeInMs();
        }

        unsigned long time = Time::getCurrentTimeInMs();
        char *commandName = splitCommandBuffer[0];

        if (returnStartTime)
        {
            if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
                strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0 ||
                strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0 ||
                strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
            {
                time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
            }
        }
        else
        {
            if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
                strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
            {

                unsigned long startTime = getMSTimeOfCommand(cmdCopy, true);
                time = startTime + atol(splitCommandBuffer[3]);
            }
        }

        return time;
    }

    bool externalCommandIsAllowed(char *commandString, bool secondary)
    {

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
        bool offline = isOffline();

        bool limitiedCmdSet = offline;
#ifdef RELAY_SUPPORTED
        if (isRelayPeer && secondary)
        {
            limitiedCmdSet = true;
        }
#endif

        if (limitiedCmdSet)
        {

            byte commandCount = 0;
            bool splitSuccess = splitIntoBuffer(commandString, commandCount);
            if (!splitSuccess)
            {
                // ignoring invalid command
                return false;
            }

            char *commandName = splitCommandBuffer[0];

            // handshake request is allowed
            if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
            {
                return true;
            }
            // modules request is allowed
            else if (strcmp_P(commandName, BasicCommands::MODULES_REQUEST) == 0)
            {
                return true;
            }
            // continue modules request is allowed
            else if (strcmp_P(commandName, BasicCommands::READY_FOR_NEXT_RESPONSE) == 0)
            {
                return true;
            }
#ifdef ENABLE_ESP_OTA_UPDATE
            // esp32 ota update is allowed
            else if (strcmp_P(commandName, BasicCommands::OTA_UPDATE) == 0)
            {
                return true;
            }
#endif
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
            // switch configuration is allowed
            else if (strcmp_P(commandName, BasicCommands::SET_CONFIG) == 0)
            {
                return true;
            }
#endif
#ifdef RELAY_SUPPORTED
            // get MAC address is allowed
            else if (strcmp_P(commandName, BasicCommands::GET_MAC_ADDRESS) == 0)
            {
                return true;
            }
#endif
            // otherwise ignore

            return false;
        }
#endif
        // allowed, not offline
        return true;
    }

    bool checkHash(char *cmdString)
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
        int hashDataEndIdx = idx;

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
        initialized = false;
        commandInProgress = false;
        serialCommandIdx = 0;
        serialCommandBuffer[serialCommandIdx] = '\0';
        timeOfLastChar = 0;
    }

    void bottangoLoop()
    {
        Callbacks::onEarlyLoop();

        updateReadBuffer(false); // standard read

#ifdef RELAY_SUPPORTED
        if (relayComs != nullptr)
        {
            relayComs->update();
        }
        if (isRelayPeer)
        {
            updateReadBuffer(true); // secondary read when peer also
        }
        if (isRelayBridge)
        {
            // was stopping all peers, and the queue is now empty
            if (relayPool->isUninitializing && relayPool->toPeerQueueEmpty())
            {
                // try stop again, and actually shut down
                // because the queue is empty, it won't abort out
                stop(true);
            }
        }
#endif

        if (initialized)
        {

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
            if (isOffline())
#endif
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
            {
                if (commandStreamProvider != nullptr)
                {
                    commandStreamProvider->updateOnLoop();
                }
            }
#endif

            // update effector pool
            effectorPool.updateAllDriveTargets();

            // update relay pool
#ifdef RELAY_SUPPORTED
            if (isRelayBridge)
            {
                relayPool->update();
            }
            else if (isRelayPeer && millis() - lastPollTimeAsPeer > RELAY_POLL_TIMEOUT_AS_PEER)
            {
                Outgoing::toggleOnSecondaryOutgoing();
                Outgoing::printOutputStringFlash(F("Lost Bridge!"));
                Outgoing::printLine();
                Outgoing::endToggleOnSecondaryOutgoing();
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

#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::updateLights();
#endif

#if defined(RELAY_SUPPORTED) && defined(RELAY_LOGGING)
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled())
#endif
        {
            if (isRelayPeer && !initialized && Time::getCurrentTimeInMs() - lastWaitForConnectLog >= 1000)
            {
                Outgoing::toggleOnSecondaryOutgoing();
                Outgoing::printOutputStringFlash(F("Waiting for bridge...\n"));
                Outgoing::endToggleOnSecondaryOutgoing();
                lastWaitForConnectLog = Time::getCurrentTimeInMs();
            }
        }
#endif

#if defined(AUDIO_SD_I2S) && defined(DYNAMIC_VOLUME)
        if (I2SHelper::isPlaying())
        {
            I2SHelper::updateVolume();
        }
#endif

#ifdef STOP_BUTTON_SUPPORTED
        if (millis() - lastStopButtonPressTime > BUTTON_DEBOUNCE_TIME)
        {
#ifdef STOP_READ_TYPE_DIGITAL
            if (digitalRead(STOP_BUTTON_PIN) == STOP_READ_ACTIVE)
            {
#elif defined(STOP_READ_TYPE_ANALOG)
            int analogVal = analogRead(STOP_BUTTON_PIN);
            if (analogVal >= STOP_READ_ACTIVE_MIN && analogVal <= STOP_READ_ACTIVE_MAX)
            {
#endif

#ifndef DYNAMIC_STOP_BUTTON_BEHAVIOR
                bool stopIsShutdown = STOP_BUTTON_SHOULD_DISCONNECT;
#else
                bool stopIsShutdown = PersistentConfigUtil::stopIsShutdown();
#endif

                if (isOffline())
                {
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
                    if (stopIsShutdown)
                    {
                        stop(true);
                    }
                    else
                    {
                        if (commandStreamProvider != nullptr)
                        {
                            commandStreamProvider->stop();
                        }
                    }
#endif
                }
                else
                {
                    if (stopIsShutdown)
                    {
                        Outgoing::outgoing_requestShutdown();
                    }
                    else
                    {
                        Outgoing::outgoing_requestStopPlay();
                    }
                }

                lastStopButtonPressTime = millis();
            }
        }
#endif
    }

    bool isOffline()
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
    }

    void updateReadBuffer(bool secondary)
    {

#if defined(USE_ESP32_WIFI)
        if (!updateWifiConnectionStatus())
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
                        if (activeOutgoingMultimessage != nullptr)
                        {
                            // send the data portion of a multi message response
                            // after ok, if any is pending
                            activeOutgoingMultimessage->emitPending();
                        }
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
#endif
    }

    bool rcvAvailable(bool secondary)
    {
#ifdef RELAY_SUPPORTED
        if (isRelayPeer)
        {
            if (secondary)
            {
                return Serial.available() > 0;
            }
            else
            {
                if (relayComs == nullptr)
                {
                    return false;
                }
                return relayComs->peerRecvAvailable();
            }
        }
        else
        {
            return Serial.available() > 0;
        }

#elif defined(USE_USB_SERIAL)
        return Serial.available() > 0;

#elif defined(USE_ESP32_WIFI)
        return client.available() > 0;

#endif
    }

    char readNextChar(bool secondary)
    {
#ifdef RELAY_SUPPORTED
        if (isRelayPeer)
        {
            if (secondary)
            {
                return Serial.read();
            }
            else
            {
                if (relayComs == nullptr)
                {
                    return '\0'; // something has gone terribly wrong, shouldn't hit this
                }
                return relayComs->peerReadNextChar();
            }
        }
        else
        {
            return Serial.read();
        }
#elif defined(USE_USB_SERIAL)
        return Serial.read();

#elif defined(USE_ESP32_WIFI)
        return client.read();
#endif
    }

} // namespace BottangoCore
