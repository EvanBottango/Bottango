#include "BottangoCore.h"

#include "../BottangoArduinoModules.h"

#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
#include "ESPNOWUtil.h"
#endif

#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"
#endif

namespace BottangoCore
{
    EffectorPool effectorPool = EffectorPool();
#ifdef RELAY_PARENT
    RelayChildPool relayPool = RelayChildPool();
#endif
    char delimiters[] = ",";

    bool initialized = false;
    bool handshake = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    CommandStreamProvider *commandStreamProvider = nullptr;
#endif

#if !defined(RELAY_CHILD) && defined(USE_ESP32_WIFI)
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

    void bottangoSetup()
    {

        // intiialize comms

#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::initLights();
        StatusLights::setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_NO_CONNECTION);
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
        StatusLights::setDesiredColor(USER_STATUS_LIGHT, CRGB::Black);
#endif

#ifdef PIN_LOW_LAUNCH
        for (int i = 0; i < PIN_REMAP_LENGTH; i++)
        {
            pinMode(onboardPins[i], OUTPUT);
            digitalWrite(onboardPins[i], LOW);
        }
#endif

#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
#ifdef ENABLE_SERIAL_LOGGING
        Serial.begin(BAUD_RATE);
#endif
        uint8_t parentAddress[] = ESPNOW_PARENT_ADDRESS;
        ESPNowUtil::initializeESPNow(parentAddress);
        Outgoing::printLine();
        Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
        Outgoing::printLine();
#elif defined(USE_USB_SERIAL)
        Serial.begin(BAUD_RATE);
        Outgoing::printOutputStringFlash(F("\n\n"));
        Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
        Outgoing::printOutputStringFlash(F("\n\n"));
#elif defined(USE_ESP32_WIFI)
#ifdef ENABLE_SERIAL_LOGGING
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

#endif

// enter exported animation if required
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
        DynamicAnimationSwitch::init();
        if (DynamicAnimationSwitch::shouldRunCommandStreams)
#endif
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
        {
#ifdef ENABLE_STATUS_LIGHTS
            StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_CONNECTION_OFFLINE);
#endif
            commandStreamProvider = new CommandStreamProvider();
            initialized = true;
            Callbacks::onThisControllerStarted();
            commandStreamProvider->runSetup();
        }
#endif
    }

#ifdef USE_ESP32_WIFI

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

#endif

    bool splitIntoBuffer(char *stringToSplit, byte &commandCount)
    {
        byte idxResult = 0;
        char *token = strtok(stringToSplit, delimiters);

#ifdef RELAY_PARENT
        // Special case if first token is relay pass through
        // should just be relayCmd, identifier, and then the rest of the string as the third token, and the hash as the last token
        if (token != NULL && strcmp_P(token, BasicCommands::PASS_TO_RELAY) == 0)
        {
            // 1) Store "sR" token
            splitCommandBuffer[idxResult++] = token;

            // 2) Store the next token (relay identifier)
            token = strtok(NULL, delimiters);
            if (token != NULL)
            {
                splitCommandBuffer[idxResult++] = token;

                // Prepare a buffer for concatenating intermediate tokens
                char passThroughCommand[MAX_COMMAND_LENGTH];
                passThroughCommand[0] = '\0';

                // Tokenize until the last token
                token = strtok(NULL, delimiters);
                while (token != NULL)
                {
                    // check if at last token (or get for next)
                    char *nextToken = strtok(NULL, delimiters);
                    if (nextToken == NULL)
                    {
                        // 3) add the buffer
                        if (passThroughCommand[0] != '\0')
                        {
                            if (passThroughCommand[strlen(passThroughCommand) - 1] == ',')
                            {
                                // Remove the trailing comma
                                passThroughCommand[strlen(passThroughCommand) - 1] = '\0';
                            }
                            splitCommandBuffer[idxResult++] = passThroughCommand;
                        }

                        // 4) Add the final token
                        splitCommandBuffer[idxResult++] = token;
                        break;
                    }

                    // add the token
                    strcat(passThroughCommand, token);
                    strcat(passThroughCommand, delimiters);

                    token = nextToken;
                }
            }
            commandCount = idxResult;
            return true;
        }
#endif

        // Regular tokenization
        while (token != NULL)
        {
            if (idxResult >= COMMANDS_PARAMS_SIZE) // Check buffer capacity
            {
                Error::reportError_TooManyParams();
                return false;
            }
            splitCommandBuffer[idxResult++] = token;
            token = strtok(NULL, delimiters);
        }

        commandCount = idxResult;
        return true;
    }

    // parses the serial buffer and turns it into commands
    // param commandString: e.g. "rSP,9,1000,3000"
    void executeCommand(char *commandString)
    {
#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::pulseSignalLight();
#endif
#ifdef ALLOW_SYNC_COMMANDS
        // before split, check if this is a syncronized command
        // we don't actually want to split a syncronized command, but to parse it's own unique syntax
        if (strncmp_P(commandString, BasicCommands::SYNC_COMMAND, 3) == 0)
        {
            BasicCommands::processSyncronizedCommands(commandString);
            return;
        }
#endif

        byte commandCount = 0;
        bool splitSuccess = splitIntoBuffer(commandString, commandCount);
        if (!splitSuccess)
        {
            return;
        }

        // The command name is the first string in the array, subsequent strings are parameters of that command
        char *commandName = splitCommandBuffer[0];

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
#ifdef RELAY_PARENT
        else if (strcmp_P(commandName, BasicCommands::PASS_TO_RELAY) == 0)
        {
            BasicCommands::passToRelayController(splitCommandBuffer, commandCount);
        }
#endif
        else if (strcmp_P(commandName, BasicCommands::DEREGISTER_ALL_EFFECTORS) == 0)
        {
            BasicCommands::deregisterAllEffectors(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
        {
            BasicCommands::sendHandshakeResponse(splitCommandBuffer);
        }
        else if (strcmp_P(commandName, BasicCommands::TIME_SYNC) == 0)
        {
            BasicCommands::syncTime(splitCommandBuffer);
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
        else if (strcmp_P(commandName, BasicCommands::REGISTER_AUDIO_EVENT) == 0)
        {
            BasicCommands::registerAudioEvent(splitCommandBuffer);
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
#ifdef RELAY_PARENT
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
#endif
#ifdef ENABLE_ESP_OTA_UPDATE
        else if (strcmp_P(commandName, BasicCommands::OTA_UPDATE) == 0)
        {
            BasicCommands::processOTA(splitCommandBuffer);
        }
#endif
    }

    unsigned long getMSStartTimeOfCommand(char *commandString)
    {
        byte commandCount = 0;
        bool splitSuccess = splitIntoBuffer(commandString, commandCount);
        if (!splitSuccess)
        {
            return Time::getCurrentTimeInMs();
        }

        unsigned long time = Time::getCurrentTimeInMs();
        char *commandName = splitCommandBuffer[0];

        if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
            strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0 ||
            strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0 ||
            strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
        {
            time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
        }
        return time;
    }

    unsigned long getMSEndTimeOfCommand(char *commandString)
    {
        unsigned long time = Time::getCurrentTimeInMs();
        byte commandCount = 0;
        bool splitSuccess = splitIntoBuffer(commandString, commandCount);
        if (!splitSuccess)
        {
            return Time::getCurrentTimeInMs();
        }

        unsigned long startTime = getMSStartTimeOfCommand(commandString);

        char *commandName = splitCommandBuffer[0];
        if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
            strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
        {
            return startTime + atol(splitCommandBuffer[3]);
        }

        return startTime;
    }

    bool externalCommandIsAllowed(char *commandString)
    {

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
        bool offline = false;
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
        if (DynamicAnimationSwitch::shouldRunCommandStreams)
        {
            offline = true;
        }
#else
        offline = true;
#endif
        if (offline)
        {
            if (handshake)
            {
                // already handshake, nothing else allowed
                return false;
            }

            byte commandCount = 0;
            bool splitSuccess = splitIntoBuffer(commandString, commandCount);
            if (!splitSuccess)
            {
                // ignoring invalid command
                return false;
            }

            char *commandName = splitCommandBuffer[0];

            // only handshake request is allowed
            if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
            {
                // Allowing handshake
                return true;
            }
            //"Non-handshake, ignoring

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

        LOG_MKBUF

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

        if (ati == hsh)
        {
            LOG_LN(F("HASH GOOD"))
            return true;
        }
        else
        {
            LOG_LN(F("HASH BAD"))
            return false;
        }
    }

    void stop()
    {
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

#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        while (ESPNowUtil::recvAvailable())
        {
            char cmd[MAX_COMMAND_LENGTH];
            ESPNowUtil::readRecv(cmd);
            int len = strlen(cmd);
            for (int i = 0; i < len; i++)
            {
                char read = cmd[i];

#elif defined(USE_USB_SERIAL)
        while (Serial.available() > 0)
        {
            char read = Serial.read();
#elif defined(USE_ESP32_WIFI)

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
            return;
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
                    return;
                }
            }
        }
        // had then lost server connection
        else if (!client.connected())
        {
            BasicCommands::stop(nullptr);
            // restart the board
            ESP.restart();
            return;
        }

        while (client.available() > 0)
        {
            char read = client.read();
#endif

                commandInProgress = true;
                timeOfLastChar = millis();

                if (read == '\n')
                {
                    LOG_MKBUF
                    LOG(F("EXECUTE! --> "))
                    LOG_LN(serialCommandBuffer)

                    bool hashPasses = checkHash(serialCommandBuffer);

                    commandInProgress = false;
                    timeOfLastChar = 0;

                    if (hashPasses)
                    {
                        if (externalCommandIsAllowed(serialCommandBuffer))
                        {
                            executeCommand(serialCommandBuffer);
                        }

                        LOG(F("t="))
                        LOG_ULONG(Time::getCurrentTimeInMs())
                        LOG_NEWLINE()

                        Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
                        serialCommandIdx = 0;
                        serialCommandBuffer[serialCommandIdx] = '\0';
                    }
                    else
                    {
                        Outgoing::printOutputStringPROGMEM(BasicCommands::HASH_FAIL);
                        serialCommandIdx = 0;
                        serialCommandBuffer[serialCommandIdx] = '\0';
                    }
                }
                else if (read == '\0')
                {
                }
                else
                {
                    if (serialCommandIdx <= MAX_COMMAND_LENGTH - 2)
                    {
                        serialCommandBuffer[serialCommandIdx++] = read;
                        serialCommandBuffer[serialCommandIdx] = '\0';
                    }
                    else
                    {
                        Error::reportError_CmdTooLong();
                        serialCommandIdx = 0;
                        serialCommandBuffer[serialCommandIdx] = '\0';

                        commandInProgress = false;
                        timeOfLastChar = 0;

                        char outStringBuffer[25];
                        Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
                        Serial.write(outStringBuffer);
                    }
                }
            }
#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        }
#endif

        if (commandInProgress && millis() - timeOfLastChar >= READ_TIMEOUT)
        {
            Outgoing::printOutputStringPROGMEM(BasicCommands::TIMEOUT);

            serialCommandIdx = 0;
            serialCommandBuffer[serialCommandIdx] = '\0';

            commandInProgress = false;
            timeOfLastChar = 0;
        }

        if (initialized)
        {
// update command streams
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
            if (DynamicAnimationSwitch::shouldRunCommandStreams)
#endif
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
            {
                commandStreamProvider->updateOnLoop();
            }
#endif

            // update effector pool
            effectorPool.updateAllDriveTargets();

// update relay pool
#ifdef RELAY_PARENT

            relayPool.update();
#endif
        }

        Callbacks::onLateLoop();

#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::updateLights();
#endif
    }
} // namespace BottangoCore
