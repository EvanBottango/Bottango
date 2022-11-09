#include "Arduino.h"
#include "BottangoCore.h"

#ifdef USE_COMMAND_STREAM
#include "../GeneratedCommandStreams.h"
#endif

namespace BottangoCore
{
    ////////////////////////////
    CommandRegistry commandRegistry = CommandRegistry();
    EffectorPool effectorPool = EffectorPool();
    CommandStreamProvider commandStreamProvider = CommandStreamProvider();

    bool initialized = false;
    bool drivePaused = false;

    char serialCommandBuffer[MAX_COMMAND_LENGTH];
    int serialCommandIdx = 0;

    unsigned long timeOfLastChar = 0;
    bool commandInProgress = false;
    ////////////////////////////

    void bottangoSetup()
    {

#ifdef ENABLE_STEPPERS
        // initialize interrupt timer on timer 2 at 10,0000 hz
        cli(); // stop interrupts

        TCCR2A = 0; // clear registers
        TCCR2B = 0;
        TCNT2 = 0;

        // 10000 Hz (16000000/((24+1)*64)) (64 prescaler and 24 ticks to reset makes 10,000 hz)
        OCR2A = 24;
        // Enable CTC
        TCCR2A |= (1 << WGM21);
        // Prescaler 64
        TCCR2B |= (1 << CS22);
        // Output Compare Match A Interrupt Enable
        TIMSK2 |= (1 << OCIE2A);

        sei(); // allow interrupts
#endif

        Serial.begin(BAUD_RATE);

        // Add the basic set of commands to the registry
        // to add a custom command, give a string for the name, and a function that takes a char *args[] (the arguments)

        commandRegistry.addCommand(BasicCommands::HANDSHAKE_REQUEST, BasicCommands::sendHandshakeResponse);
        commandRegistry.addCommand(BasicCommands::TIME_SYNC, BasicCommands::syncTime);
        commandRegistry.addCommand(BasicCommands::COMMAND_SEQUENCE_BEGIN, BasicCommands::pauseDrive);
        commandRegistry.addCommand(BasicCommands::COMMAND_SEQUENCE_END, BasicCommands::unpauseDrive);

        commandRegistry.addCommand(BasicCommands::REGISTER_I2C_SERVO, BasicCommands::registerI2CServo);
        commandRegistry.addCommand(BasicCommands::REGISTER_PIN_SERVO, BasicCommands::registerPinServo);
        commandRegistry.addCommand(BasicCommands::REGISTER_PIN_STEPPER, BasicCommands::registerPinStepper);
        commandRegistry.addCommand(BasicCommands::REGISTER_DIR_STEPPER, BasicCommands::registerDirStepper);
        commandRegistry.addCommand(BasicCommands::REGISTER_I2C_STEPPER, BasicCommands::registerI2CStepper);
        commandRegistry.addCommand(BasicCommands::REGISTER_CURVED_EVENT, BasicCommands::registerCurvedEvent);
        commandRegistry.addCommand(BasicCommands::REGISTER_ONOFF_EVENT, BasicCommands::registerOnOffEvent);
        commandRegistry.addCommand(BasicCommands::REGISTER_TRIGGER_EVENT, BasicCommands::registerTriggerEvent);
        commandRegistry.addCommand(BasicCommands::REGISTER_CUSTOM_MOTOR, BasicCommands::registerCustomMotor);
        commandRegistry.addCommand(BasicCommands::REGISTER_COLOR_EVENT, BasicCommands::registerColorEvent);

        commandRegistry.addCommand(BasicCommands::DEREGISTER_EFFECTOR, BasicCommands::deregisterEffector);
        commandRegistry.addCommand(BasicCommands::DEREGISTER_ALL_EFFECTORS, BasicCommands::deregisterAllEffectors);

        commandRegistry.addCommand(BasicCommands::CLEAR_ALL_CURVES, BasicCommands::clearAllCurves);
        commandRegistry.addCommand(BasicCommands::SET_CURVE, BasicCommands::addCurve);
        commandRegistry.addCommand(BasicCommands::SET_INSTANTCURVE, BasicCommands::addInstantCurve);
        commandRegistry.addCommand(BasicCommands::SET_ONOFFCURVE, BasicCommands::addOnOffCurve);
        commandRegistry.addCommand(BasicCommands::SET_TRIGGERCURVE, BasicCommands::addTriggerCurve);
        commandRegistry.addCommand(BasicCommands::CLEAR_EFFECTOR_CURVES, BasicCommands::clearCurvesForEffector);

        commandRegistry.addCommand(BasicCommands::SET_COLOR_CURVE, BasicCommands::addColorCurve);
        commandRegistry.addCommand(BasicCommands::SET_INSTANT_COLOR_CURVE, BasicCommands::addInstantColorCurve);

        commandRegistry.addCommand(BasicCommands::MANUAL_SYNC, BasicCommands::manualSync);

        Serial.print(F("\n\n"));
        BasicCommands::printOutputString(BasicCommands::BOOT);
        Serial.print(F("\n\n"));

#ifdef USE_COMMAND_STREAM
        commandStreamProvider.runSetup();
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
        }
        else
        {
            LOG_LN(F("HASH BAD"))
        }

        return hsh;
    }

    void bottangoLoop()
    {
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

                if (hashPasses)
                {
                    if (commandRegistry.externalCommandIsValid(serialCommandBuffer))
                    {
                        commandRegistry.executeCommand(serialCommandBuffer);
                    }

                    LOG(F("t="))
                    LOG_ULONG(Time::getCurrentTimeInMs())
                    LOG_NEWLINE()

                    BasicCommands::printOutputString(BasicCommands::READY);
                }
                else
                {
                    BasicCommands::printOutputString(BasicCommands::HASH_FAIL);
                }

                serialCommandIdx = 0;
                serialCommandBuffer[serialCommandIdx] = '\0';

                commandInProgress = false;
                timeOfLastChar = 0;
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
            BasicCommands::printOutputString(BasicCommands::HASH_FAIL);

            serialCommandIdx = 0;
            serialCommandBuffer[serialCommandIdx] = '\0';

            commandInProgress = false;
            timeOfLastChar = 0;
        }

#ifdef USE_COMMAND_STREAM
        commandStreamProvider.updateOnLoop();
#endif

        if (!drivePaused)
        {
            effectorPool.updateAllDriveTargets();
        }
    }

#ifdef ENABLE_STEPPERS
    ISR(TIMER2_COMPA_vect)
    {
        // drive interrupt loop on timer 2
        effectorPool.interruptDriveLoop();
    }
#endif
} // namespace BottangoCore
