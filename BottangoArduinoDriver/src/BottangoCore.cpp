#include "BottangoCore.h"

#ifdef USE_COMMAND_STREAM
#include "../GeneratedCommandStreams.h"
#endif

namespace BottangoCore
{
    EffectorPool effectorPool = EffectorPool();
    bool initialized = false;
#ifdef USE_COMMAND_STREAM
    CommandStreamProvider commandStreamProvider = CommandStreamProvider();
#endif

    char serialCommandBuffer[MAX_COMMAND_LENGTH];
    int serialCommandIdx = 0;

    unsigned long timeOfLastChar = 0;
    bool commandInProgress = false;
    char *splitCommandBuffer[COMMANDS_PARAMS_SIZE];

    void bottangoSetup()
    {
        Serial.begin(BAUD_RATE);

        Serial.print(F("\n\n"));
        BasicCommands::printOutputString(BasicCommands::BOOT);
        Serial.print(F("\n\n"));

#ifdef USE_COMMAND_STREAM
        initialized = true;
        commandStreamProvider.runSetup();
#endif
    }

    bool splitIntoBuffer(char *stringToSplit)
    {
        byte idxResult = 0;
        char *wordStart;
        char delimiters[] = ",";

        if (idxResult + 1 >= COMMANDS_PARAMS_SIZE)
        {
            Error::reportError_TooManyParams();
            return false;
        }

        wordStart = strtok(stringToSplit, delimiters);
        while (wordStart != NULL)
        {
            splitCommandBuffer[idxResult++] = wordStart;
            wordStart = strtok(NULL, delimiters);
        }

        splitCommandBuffer[idxResult] = ((char *)"\0");

        return true;
    }

    // parses the serial buffer and turns it into commands
    // param commandString: e.g. "rSP,9,1000,3000"
    void executeCommand(char *commandString)
    {
        bool splitSuccess = splitIntoBuffer(commandString);
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
    }

    unsigned long getMSTimeOfCommand(char *commandString)
    {
        unsigned long time = Time::getCurrentTimeInMs();
        bool splitSuccess = splitIntoBuffer(commandString);
        if (!splitSuccess)
        {
            return 0;
        }

        char *commandName = splitCommandBuffer[0];

        // only curves have time different than now, so parse the string to find the time of each curve type
        if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0)
        {
            time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0)
        {
            time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
        }
        else if (strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0)
        {
            time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
        }

        return time;
    }

    bool externalCommandIsValid(char *commandString)
    {
#ifndef USE_COMMAND_STRING
        return true;
#else
        bool splitSuccess = splitIntoBuffer(commandString);
        if (!splitSuccess)
        {
            return false;
        }

        char *commandName = splitCommandBuffer[0];

        // only handshake request is allowed
        if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
        {
            return true;
        }

        return false;
#endif
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

        while (Serial.available() > 0)
        {
            char read = Serial.read();

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
                    if (externalCommandIsValid(serialCommandBuffer))
                    {
                        executeCommand(serialCommandBuffer);
                    }

                    LOG(F("t="))
                    LOG_ULONG(Time::getCurrentTimeInMs())
                    LOG_NEWLINE()

                    BasicCommands::printOutputString(BasicCommands::READY);
                    serialCommandIdx = 0;
                    serialCommandBuffer[serialCommandIdx] = '\0';
                }
                else
                {
                    BasicCommands::printOutputString(BasicCommands::HASH_FAIL);
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
                    BasicCommands::printOutputString(BasicCommands::READY);
                    Serial.write(outStringBuffer);
                }
            }
        }

        if (commandInProgress && millis() - timeOfLastChar >= READ_TIMEOUT)
        {
            BasicCommands::printOutputString(BasicCommands::TIMEOUT);

            serialCommandIdx = 0;
            serialCommandBuffer[serialCommandIdx] = '\0';

            commandInProgress = false;
            timeOfLastChar = 0;
        }

        if (initialized)
        {
#ifdef USE_COMMAND_STREAM
            commandStreamProvider.updateOnLoop();
#endif
            effectorPool.updateAllDriveTargets();
        }

        Callbacks::onLateLoop();
    }
} // namespace BottangoCore
