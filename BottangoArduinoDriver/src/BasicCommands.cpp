
#include "BasicCommands.h"
#include "PinServoEffector.h"
#include "PinStepperEffector.h"
#include "I2CServoEffector.h"
#include "StepDirStepperEffector.h"
#include "CurvedCustomEvent.h"
#include "ColorCustomEvent.h"
#include "OnOffCustomEvent.h"
#include "OnOffCurve.h"
#include "TriggerCustomEvent.h"
#include "TriggerCurve.h"
#include "ColorCurve.h"
#include "CustomMotorEffector.h"
#include "Outgoing.h"
#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#endif

#ifdef RELAY_PARENT
#include "RelayChild.h"
#endif

#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"
#endif

#ifdef ENABLE_ESP_OTA_UPDATE
#include "OTAUpdateUtil.h"
#endif

namespace BasicCommands
{
    // [0] command, [1] driver version, [2]  hash code, [3] accepting incoming commands
    void sendHandshakeResponse(char *args[])
    {
        bool offlinePlayback = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
        if (DynamicAnimationSwitch::shouldRunCommandStreams)
        {
            offlinePlayback = true;
        }
#else
        offlinePlayback = true;
#endif
#endif

        if (BottangoCore::initialized)
        {
            LOG_LN(F("WARN: already init"))
        }
        if (BottangoCore::handshake)
        {
            LOG_LN(F("WARN: already handshake"))
        }
        LOG_LN(F("sendHandshakeResponse"))

        BottangoCore::initialized = true;
        BottangoCore::handshake = true;

        if (!offlinePlayback)
        {
            deregisterAllEffectors(NULL);
        }

        char *code = args[1];

        // command name
        Outgoing::printOutputStringPROGMEM(BasicCommands::HANDSHAKE);
        Outgoing::printOutputStringFlash(F(","));

        // driver version
        Outgoing::printOutputStringPROGMEM(BasicCommands::DRIVER_VERSION);
        Outgoing::printOutputStringFlash(F(","));

        // repeat back hash code
        Outgoing::printOutputStringMem(code);
        Outgoing::printOutputStringFlash(F(","));

        // true if accepting incoming commands, false if not (IE offline playback)
        if (offlinePlayback)
        {
            Outgoing::printOutputStringFlash(F("0"));
        }
        else
        {
            Outgoing::printOutputStringFlash(F("1"));
        }

        Outgoing::printOutputStringFlash(F("\n"));

        Serial.flush();

#ifdef ENABLE_STATUS_LIGHTS
#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_HAS_CONNECTION);
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_CHILD);
#elif defined(USE_USB_SERIAL)
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_HAS_CONNECTION);
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_SERIAL);
#endif
#endif

        Callbacks::onThisControllerStarted();
    }

    void stop(char *args[])
    {
        BottangoCore::effectorPool.deregisterAll();
        BottangoCore::stop();
#ifdef ENABLE_STATUS_LIGHTS
        StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_LOST_CONNECTION);
        StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
#endif
        Callbacks::onThisControllerStopped();
    }

    void syncTime(char *args[])
    {
        Time::syncTime(atol(args[1]));
    }

    void deregisterAllEffectors(char **args)
    {
        LOG_LN(F("deregister all"));
        BottangoCore::effectorPool.deregisterAll();
    }

    void clearAllCurves(char **args)
    {
        BottangoCore::effectorPool.clearAllCurves();
    }

    void updateEffectorSignalBounds(char **args)
    {
        char *identifier = args[1];
        int minSignal = atoi(args[2]);
        int maxSignal = atoi(args[3]);
        int signalSpeed = atoi(args[4]);

        BottangoCore::effectorPool.updateEffectorSignalBounds(identifier, minSignal, maxSignal, signalSpeed);
    }

    void registerPinServo(char **args)
    {

        byte pinId = atoi(args[1]);
        int minPWM = atoi(args[2]);
        int maxPWM = atoi(args[3]);
        int maxPWMSec = atoi(args[4]);
        int startPWM = atoi(args[5]);

        LOG_MKBUF
        LOG_LN(F("registerServo()"))
        LOG(F("    pinId="))
        LOG_INT(pinId)
        LOG(F("    minPWM="))
        LOG_INT(minPWM)
        LOG(F("    maxPWM="))
        LOG_INT(maxPWM)
        LOG(F("    startPWM="))
        LOG_INT(startPWM)
        LOG_NEWLINE()

        PinServoEffector *newEffector = new PinServoEffector(pinId, minPWM, maxPWM, maxPWMSec, startPWM);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerI2CServo(char **args)
    {

#ifndef USE_ADAFRUIT_PWM_LIBRARY
        Error::reportError_MissingLibrary();
        return;
#endif

        byte address = atoi(args[1]);
        byte pinId = atoi(args[2]);
        int minPWM = atoi(args[3]);
        int maxPWM = atoi(args[4]);
        int maxPWMSec = atoi(args[5]);
        int startPWM = atoi(args[6]);

        LOG_MKBUF
        LOG_LN(F("register i2c Servo()"))
        LOG(F("    address="))
        LOG_INT(address)
        LOG(F("    pinId="))
        LOG_INT(pinId)
        LOG(F("    minPWM="))
        LOG_INT(minPWM)
        LOG(F("    maxPWM="))
        LOG_INT(maxPWM)
        LOG(F("    startPWM="))
        LOG_INT(startPWM)
        LOG_NEWLINE()

        I2CServoEffector *newEffector = new I2CServoEffector(address, pinId, minPWM, maxPWM, maxPWMSec, startPWM);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerPinStepper(char **args)
    {

        byte pin0 = atoi(args[1]);
        byte pin1 = atoi(args[2]);
        byte pin2 = atoi(args[3]);
        byte pin3 = atoi(args[4]);

        int maxCounterClockwiseSteps = atoi(args[5]);
        int maxClockwiseSteps = atoi(args[6]);
        int maxStepsPerSecond = atoi(args[7]);
        int startingStepOffset = atoi(args[8]);

        LOG_MKBUF
        LOG_LN(F("register pin stepper()"))
        LOG(F("    pin0="))
        LOG_INT(pin0)
        LOG(F("    pin1="))
        LOG_INT(pin1)
        LOG(F("    pin2="))
        LOG_INT(pin2)
        LOG(F("    pin3="))
        LOG_INT(pin3)
        LOG(F("    maxCC="))
        LOG_INT(maxCounterClockwiseSteps)
        LOG(F("    maxC="))
        LOG_INT(maxClockwiseSteps)
        LOG(F("    speed="))
        LOG_INT(maxStepsPerSecond)
        LOG_NEWLINE()

        PinStepperEffector *newEffector = new PinStepperEffector(pin0, pin1, pin2, pin3, maxCounterClockwiseSteps, maxClockwiseSteps, maxStepsPerSecond, startingStepOffset);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerDirStepper(char **args)
    {

        byte stepPin = atoi(args[1]);
        byte dirPin = atoi(args[2]);

        bool clockwiseIsLow = atoi(args[3]) != 0;

        int maxCounterClockwiseSteps = atoi(args[4]);
        int maxClockwiseSteps = atoi(args[5]);
        int maxStepsPerSecond = atoi(args[6]);
        int startingStepOffset = atoi(args[7]);

        LOG_MKBUF
        LOG_LN(F("register dir stepper()"))
        LOG(F("    step="))
        LOG_INT(stepPin)
        LOG(F("    dir="))
        LOG_INT(dirPin)
        LOG(F("    maxCC="))
        LOG_INT(maxCounterClockwiseSteps)
        LOG(F("    maxC="))
        LOG_INT(maxClockwiseSteps)
        LOG(F("    speed="))
        LOG_INT(maxStepsPerSecond)
        LOG_NEWLINE()

        StepDirStepperEffector *newEffector = new StepDirStepperEffector(stepPin, dirPin, clockwiseIsLow, maxCounterClockwiseSteps, maxClockwiseSteps, maxStepsPerSecond, startingStepOffset);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerCurvedEvent(char **args)
    {
        char *identifier = args[1];
        float maxSpeed = atof(args[2]);
        float startingMovement = atof(args[3]);
        byte pin = atoi(args[4]);

        LOG_MKBUF
        LOG_LN(F("register curved event"))
        LOG(F("    id="))
        LOG(identifier)
        LOG_NEWLINE()

        CurvedCustomEvent *newEffector = new CurvedCustomEvent(identifier, maxSpeed, startingMovement, pin);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerOnOffEvent(char **args)
    {
        char *identifier = args[1];
        bool startOn = atoi(args[2]) != 0;
        byte pin = atoi(args[3]);

        LOG_MKBUF
        LOG_LN(F("register on off event"))
        LOG(F("    id="))
        LOG(identifier)
        LOG_NEWLINE()

        OnOffCustomEvent *newEffector = new OnOffCustomEvent(identifier, startOn, pin);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerTriggerEvent(char **args)
    {
        char *identifier = args[1];
        byte pin = atoi(args[2]);
        bool fireIsHigh = atoi(args[3]) != 0;

        LOG_MKBUF
        LOG_LN(F("register trigger event"))
        LOG(F("    id="))
        LOG(identifier)
        LOG_NEWLINE()

        TriggerCustomEvent *newEffector = new TriggerCustomEvent(identifier, pin, fireIsHigh);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerAudioEvent(char **args)
    {
        char *identifier = args[1];
        byte index = atoi(args[2]);

#ifdef AUDIO_TRIGGER_EVENT
        // should not be hit
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errInvalidAudioKeyframe"));
        Outgoing::printLine();
#elif defined(AUDIO_SD_I2S)
        LOG_MKBUF
        LOG_LN(F("register audio i2s event"))
        LOG(F("    id="))
        LOG(identifier)
        LOG_NEWLINE()

        I2SAudioEffector *newEffector = new I2SAudioEffector(identifier, index);
        BottangoCore::effectorPool.addEffector(newEffector);
#endif
    }

    void registerColorEvent(char **args)
    {
        char *identifier = args[1];

        byte r = atoi(args[2]);
        byte g = atoi(args[3]);
        byte b = atoi(args[4]);

        LOG_MKBUF
        LOG_LN(F("register color event"))
        LOG(F("    id="))
        LOG(identifier)
        LOG_NEWLINE()

        ColorCustomEvent *newEffector = new ColorCustomEvent(identifier, r, g, b);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void registerCustomMotor(char **args)
    {
        char *identifier = args[1];
        int minSignal = atoi(args[2]);
        int maxSignal = atoi(args[3]);
        int maxSignalSec = atoi(args[4]);
        int startSignal = atoi(args[5]);

        LOG_MKBUF
        LOG_LN(F("register custom motor"))
        LOG(F("    identifier="))
        LOG(identifier)
        LOG(F("    minSignal="))
        LOG_INT(minSignal)
        LOG(F("    maxSignal="))
        LOG_INT(maxSignal)
        LOG(F("    startSignal="))
        LOG_INT(startSignal)
        LOG_NEWLINE()

        CustomMotorEffector *newEffector = new CustomMotorEffector(identifier, minSignal, maxSignal, maxSignalSec, startSignal);
        BottangoCore::effectorPool.addEffector(newEffector);
    }

    void deregisterEffector(char **args)
    {
        char *identifier = args[1];

        LOG_MKBUF
        LOG_LN(F("deregister"))
        LOG(F("    identifier="))
        LOG(identifier)
        LOG_NEWLINE()

        BottangoCore::effectorPool.removeEffector(identifier);
    }

    /**
     * Command to set a curve on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2] duration of curve, [3] startY, [4] startControlX, [5] startControlY,
     * [6] endY, [7] endControlX, [8] endControlY,
     */

    void addCurve(char **args)
    {
        char *identifier = args[1];

        // in order to keep the command short in char length, some shortcuts are taken in the string sent for a curve:

        // start is time in MS since last time sync
        long startXParsed = atol(args[2]);
        unsigned long startX = Time::getLastSyncedTimeInMs();
        if (startXParsed < 0 && startXParsed * -1 > startX)
        {
            startX = 0;
        }
        else
        {
            startX += startXParsed;
        }

        // end is duration of curve
        long duration = atol(args[3]);

        // start control x is relative to start
        long startControlX = atol(args[5]);

        // end control x is relative to end
        long endControlX = atol(args[8]);

        // start Y is int 0 - COMPRESSED_SIGNAL_MAX
        int startY = atoi(args[4]);

        // start control Y is int 0 - COMPRESSED_SIGNAL_MAX
        int startControlY = atoi(args[6]);

        // end Y is int 0 - COMPRESSED_SIGNAL_MAX
        int endY = atoi(args[7]);

        // end control Y is int 0 - COMPRESSED_SIGNAL_MAX
        int endControlY = atoi(args[9]);

        if (BottangoCore::effectorPool.effectorUsesFloatCurve(identifier))
        {
            BottangoCore::effectorPool.addCurveToEffector(identifier, new FloatBezierCurve(startX, duration, startY, startControlX, startControlY, endY, endControlX, endControlY));
        }
        else
        {
            BottangoCore::effectorPool.addCurveToEffector(identifier, new FixedBezierCurve(startX, duration, startY, startControlX, startControlY, endY, endControlX, endControlY));
        }
    }

    /**
     * Command to set an instant and immediate curve on an effector with an
     * [1]identifier, [2] targetMovement,
     */
    void addInstantCurve(char **args)
    {
        char *identifier = args[1];

        // end movement is int 0 - COMPRESSED_SIGNAL_MAX
        int endMovement = atoi(args[2]);

        LOG_MKBUF
        LOG_LN(F("addInstantCurve"));

        LOG(F("    identifier="))
        LOG(identifier)

        LOG(F("    endMovement("))
        LOG(args[2])
        LOG(F(")="))
        LOG_INT(endMovement)

        if (BottangoCore::effectorPool.effectorUsesFloatCurve(identifier))
        {
            BottangoCore::effectorPool.addCurveToEffector(identifier, new FloatBezierCurve(Time::getCurrentTimeInMs(), 0, endMovement, 0, 0, endMovement, 0, 0));
        }
        else
        {
            BottangoCore::effectorPool.addCurveToEffector(identifier, new FixedBezierCurve(Time::getCurrentTimeInMs(), 0, endMovement, 0, 0, endMovement, 0, 0));
        }
    }

    /**
     * Command to set a on/off on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2]on/off
     */
    void addOnOffCurve(char **args)
    {
        char *identifier = args[1];

        // in order to keep the command short in char length, some shortcuts are taken in the string sent for a curve:

        // start is time in MS since last time sync
        long startTimeParsed = atol(args[2]);
        unsigned long startTime = Time::getLastSyncedTimeInMs();
        if (startTimeParsed < 0 && startTimeParsed * -1 > startTime)
        {
            startTime = 0;
        }
        else
        {
            startTime += startTimeParsed;
        }

        bool on = atoi(args[3]) != 0;

        LOG_MKBUF
        LOG_LN(F("addOnOffCurve"));

        LOG(F("    last Sync="))
        LOG_ULONG(Time::getLastSyncedTimeInMs())

        LOG(F("    identifier="))
        LOG(identifier)

        LOG(F("    startTime("))
        LOG(args[2])
        LOG(F(")="))
        LOG_ULONG(startTime)

        LOG(F("    on("))
        LOG(args[3])
        LOG(F(")="))
        LOG(on)
        LOG_NEWLINE()

        BottangoCore::effectorPool.addCurveToEffector(identifier, new OnOffCurve(startTime, on));
    }

    /**
     * Command to set a on/off on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2]on/off
     */
    void addTriggerCurve(char **args)
    {
        char *identifier = args[1];

        // in order to keep the command short in char length, some shortcuts are taken in the string sent for a curve:

        // start is time in MS since last time sync
        long startTimeParsed = atol(args[2]);
        unsigned long startTime = Time::getLastSyncedTimeInMs();
        if (startTimeParsed < 0 && startTimeParsed * -1 > startTime)
        {
            startTime = 0;
        }
        else
        {
            startTime += startTimeParsed;
        }
#ifdef AUDIO_SD_I2S
        unsigned long offset = atol(args[3]);
        BottangoCore::effectorPool.addCurveToEffector(identifier, new TriggerCurve(startTime, offset));
#else
        BottangoCore::effectorPool.addCurveToEffector(identifier, new TriggerCurve(startTime));
#endif

        LOG_MKBUF
        LOG_LN(F("addTriggerCurve"));

        LOG(F("    last Sync="))
        LOG_ULONG(Time::getLastSyncedTimeInMs())

        LOG(F("    identifier="))
        LOG(identifier)

        LOG(F("    startTime("))
        LOG(args[2])
        LOG(F(")="))
        LOG_ULONG(startTime)

        LOG_NEWLINE()
    }

    void addColorCurve(char **args)
    {
        char *identifier = args[1];

        // in order to keep the command short in char length, some shortcuts are taken in the string sent for a curve:

        // start is time in MS since last time sync
        long startXParsed = atol(args[2]);
        unsigned long startX = Time::getLastSyncedTimeInMs();
        if (startXParsed < 0 && startXParsed * -1 > startX)
        {
            startX = 0;
        }
        else
        {
            startX += startXParsed;
        }

        // end is duration of curve
        unsigned long duration = atol(args[3]);

        int startR = atoi(args[4]);
        int startG = atoi(args[5]);
        int startB = atoi(args[6]);

        int endR = atoi(args[7]);
        int endG = atoi(args[8]);
        int endB = atoi(args[9]);

        LOG_MKBUF
        LOG_LN(F("addColorCurve"));

        LOG(F("    last Sync="))
        LOG_ULONG(Time::getLastSyncedTimeInMs())

        LOG(F("    identifier="))
        LOG(identifier)

        LOG(F("    startTime("))
        LOG(args[2])
        LOG(F(")="))
        LOG_ULONG(startX)

        LOG(F("    duration("))
        LOG(args[3])
        LOG(F(")="))
        LOG_ULONG(duration)

        LOG(F("    startColor("))
        LOG_INT(startR)
        LOG(F(","))
        LOG_INT(startG)
        LOG(F(","))
        LOG_INT(startB)
        LOG(F(")="))

        LOG(F("    endColor("))
        LOG_INT(endR)
        LOG(F(","))
        LOG_INT(endG)
        LOG(F(","))
        LOG_INT(endB)
        LOG(F(")="))

        BottangoCore::effectorPool.addCurveToEffector(identifier, new ColorCurve(startX, duration, startR, startG, startB, endR, endG, endB));
    }

    /**
     * Command to set an instant and immediate curve on an effector with an
     * [1]identifier, [2] targetR, [3] targetG, [4] targetB
     */
    void addInstantColorCurve(char **args)
    {
        char *identifier = args[1];

        int r = atoi(args[2]);
        int g = atoi(args[3]);
        int b = atoi(args[4]);

        LOG_MKBUF
        LOG_LN(F("addInstColorCrve"));

        LOG(F("    identifier="))
        LOG(identifier)

        LOG(F("    endColor("))
        LOG_INT(args[2])
        LOG(F(","))
        LOG_INT(args[3])
        LOG(F(","))
        LOG_INT(args[4])
        LOG(F(")="))

        BottangoCore::effectorPool.addCurveToEffector(identifier, new ColorCurve(Time::getCurrentTimeInMs(), 0, r, g, b, r, g, b));
    }

    /**
     * Command to sync a stepper type effector
     * [1]identifier, [2] targetSync (0 - 100 and 0 - -100 are manual sync) ( any number > 100 and <-100 are auto sync in the given direction)
     */
    void stepperSync(char **args)
    {
        char *identifier = args[1];
        int syncVal = atoi(args[2]);

        LOG_MKBUF
        LOG_LN(F("manual sync"))
        LOG(F("    identifier="))
        LOG(identifier)
        LOG_NEWLINE()

        BottangoCore::effectorPool.syncEffector(identifier, syncVal);
    }

    void clearCurvesForEffector(char **args)
    {
        char *identifier = args[1];

        LOG_MKBUF
        LOG_LN(F("clearCurvesForServo()"));

        LOG(F("    pinId="))
        LOG(identifier)

        BottangoCore::effectorPool.clearCurvesForEffector(identifier);
    }
#ifdef RELAY_PARENT
    void registerRelayController(char **args)
    {
        char *identifier = args[1];
        // ignore type for now, which is token [2]
        char *macAddress = args[3];

        RelayChild *newRelay = new RelayChild(identifier, macAddress);
        BottangoCore::relayPool.addRelay(newRelay);
    }

    void deregisterRelayController(char **args)
    {
        char *identifier = args[1];
        BottangoCore::relayPool.removeRelay(identifier);
    }

    void deregisterAllRelayControllers(char **args)
    {
        BottangoCore::relayPool.deregisterAll();
    }

    void passToRelayController(char **args, byte commandCount)
    {
        char *identifier = args[1];
        BottangoCore::relayPool.passThroughCommandToRelay(identifier, args, commandCount);
    }
#endif

#ifdef ALLOW_SYNC_COMMANDS
    void processSyncronizedCommands(char *args)
    {
        char workingCommand[MAX_COMMAND_LENGTH];
        int workingCommandCursor = 0;
        workingCommand[0] = '\0';
        char cmdID[15];
        cmdID[0] = '\0';

        int length = strlen(args);
        int cmdIdLength = 0;
        bool firstTokenSkipped = false;

        for (int i = 0; i < length; i++)
        {

            char c = args[i];

            // skip the first token which is "sSY,"
            if (!firstTokenSkipped)
            {
                if (c == BottangoCore::delimiters[0])
                {
                    firstTokenSkipped = true;
                }
                continue;
            }

            bool skipAddChar = false; // so we don't add the last char of the cmd id twice
            // find the command ID
            if (cmdID[0] == '\0')
            {
                skipAddChar = true;
                while (i < length)
                {
                    cmdID[cmdIdLength] = c;
                    cmdIdLength++;

                    i++;
                    c = args[i];
                    // break at "," end of command id
                    // include "," so it's command id and deliniator is appended in each reconstructed command
                    if (c == BottangoCore::delimiters[0])
                    {
                        cmdID[cmdIdLength] = c;
                        cmdIdLength++;
                        cmdID[cmdIdLength] = '\0';
                        break;
                    }
                }
            }

            // add the commandID to the working string
            // at the beginning of the working string
            // and advance working cursor
            if (workingCommandCursor == 0)
            {
                for (int j = 0; j < cmdIdLength; j++)
                {
                    workingCommand[workingCommandCursor] = cmdID[j];
                    workingCommandCursor++;
                }
                if (skipAddChar)
                {
                    continue;
                }
            }

            // reached end of command
            if (c == ';')
            {
                // don't include ; but instead
                workingCommand[workingCommandCursor] = '\n'; // newline
                workingCommandCursor++;
                workingCommand[workingCommandCursor] = '\0';  // null terminate
                BottangoCore::executeCommand(workingCommand); // execute the command

                // reset
                workingCommand[0] = '\0';
                workingCommandCursor = 0;
            }
            // reached new command type
            else if (c == '*')
            {
                // reset command ID
                cmdID[0] = '\0';
                cmdIdLength = 0;
            }
            // add data to working string
            else
            {
                workingCommand[workingCommandCursor] = c;
                workingCommandCursor++;
                workingCommand[workingCommandCursor] = '\0';
            }
        }
    }
#endif

#ifdef ENABLE_ESP_OTA_UPDATE
    void processOTA(char **args)
    {
        char otaMessageType = args[1][0];

        if (otaMessageType == 's')
        {
            OTAUpdateUtil::beginOTA();
        }
        else if (otaMessageType == 'd')
        {
            OTAUpdateUtil::recvOTAData(args[2]);
        }
        else if (otaMessageType == 'e')
        {
            OTAUpdateUtil::finishOTA(args[2]);
        }
    }
#endif

} // namespace BasicCommands
