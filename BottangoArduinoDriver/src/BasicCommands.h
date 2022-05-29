#ifndef BOTTANGO_BASICCOMMANDS_H
#define BOTTANGO_BASICCOMMANDS_H

#include "CommandRegistry.h"
#include "Log.h"
#include "EffectorPool.h"
#include "Errors.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"

/** This is the set of basic supported commands, plus a loop() function that reads serial input and executes commands */
namespace BasicCommands
{
    /** !!!!!!!!!! */
    /** INCOMING STRINGS */
    /** !!!!!!!!!! */
    /** !!!!!!!!!! */

    /** Request to establish that the serial port is still open */
    const char HANDSHAKE_REQUEST[] PROGMEM = "hRQ";
    const char COMMAND_SEQUENCE_BEGIN[] PROGMEM = "@";
    const char COMMAND_SEQUENCE_END[] PROGMEM = "<";
    const char TIME_SYNC[] PROGMEM = "tSYN";

    /** Remove all registered effectors */
    const char DEREGISTER_ALL_EFFECTORS[] PROGMEM = "xE";

    /** Deregister a Servo type effector with a [0]pin */
    const char DEREGISTER_EFFECTOR[] PROGMEM = "xUE";

    /** Remove all active curves */
    const char CLEAR_ALL_CURVES[] PROGMEM = "xC";

    /** Clear all curves from effector with the given [0]pin */
    const char CLEAR_EFFECTOR_CURVES[] PROGMEM = "xUC";

    /** Register a Servo type effector with a [0]pin, [1] minPWM, [2] maxPWM, [3] maxPWMPerSec, [4] startingPWM */
    const char REGISTER_PIN_SERVO[] PROGMEM = "rSVPin";

    /** Register a Servo type effector with a [0] i2c address [1]pin, [2] minPWM, [3] maxPWM, [4] maxPWMPerSec, [5] startingPWM */
    const char REGISTER_I2C_SERVO[] PROGMEM = "rSVI2C";

    /** Register a Stepper type effector with a [0]pin0, [1]pin1, [2]pin2, [3]pin3, [4]maxCounterClockwiseSteps, [5]maxClockwiseSteps, [6]maxStepsPerSecond */
    const char REGISTER_PIN_STEPPER[] PROGMEM = "rSTPin";

    /** Register a Stepper type effector with a [0] step Pin, [1] direction Pin, [2] should clockwise on Low, [3]maxCounterClockwiseSteps, [4]maxClockwiseSteps, [5]maxStepsPerSecond, [6] startingStepOffset */
    const char REGISTER_DIR_STEPPER[] PROGMEM = "rSTDir";

    /** Register a Stepper type effector with a [0] address, [1] Pin, [2] maxCounterClockwiseSteps, [3]maxClockwiseSteps, [4]maxStepsPerSecond, [5] startingStepOffset */
    const char REGISTER_I2C_STEPPER[] PROGMEM = "rSTi2c";

    /** Register a Curved Custom Event type effector with a [0] identifier, [1] max movement per second, [2] starting movement */
    const char REGISTER_CURVED_EVENT[] PROGMEM = "rECC";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting on */
    const char REGISTER_ONOFF_EVENT[] PROGMEM = "rECOnOff";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting on */
    const char REGISTER_TRIGGER_EVENT[] PROGMEM = "rECTrig";

    /** Register a custom motor with an [0]identifier, [1] minSignal, [2] maxSignal, [3] maxSignalPerSec, [4] startingSignal */
    const char REGISTER_CUSTOM_MOTOR[] PROGMEM = "rMTR";

    /**
     * Command to set a curve on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2] duration of curve, [3] startY, [4] startControlX, [5] startControlY,
     * [6] endY, [7] endControlX, [8] endControlY,
     */
    const char SET_CURVE[] PROGMEM = "sC";

    /**
     * Command to set an instant curve on an effector with an
     * [0]identifier, [1] start time relative to last sync, [2] targetMovement,
     */
    const char SET_INSTANTCURVE[] PROGMEM = "sCI";

    /**
     * Command to set a on/off on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2]on/off
     */
    const char SET_ONOFFCURVE[] PROGMEM = "sCO";

    /**
     * Command to set a trigger on an effector with an
     * [0]identifier, [1] startX relative to last sync,
     */
    const char SET_TRIGGERCURVE[] PROGMEM = "sCT";

    /**
     * Command to change motor position in order to sync, without using movement
     * [0]identifier, [1] syncValue
     */
    const char MANUAL_SYNC[] PROGMEM = "sycM";

    /** !!!!!!!!!! */
    /** OUTGOING STRINGS */
    /** !!!!!!!!!! */
    /** !!!!!!!!!! */

    /// outgoing command strings, these will be sent back to Bot Tango from the Arudino
    /** The arduino has (re-)started */
    const char BOOT[] PROGMEM = "BOOT";

    /** Confirmation that the serial port opened is the correct one */
    const char HANDSHAKE[] PROGMEM = "btngoHSK";

    /** The version code of this driver */
    const char DRIVER_VERSION[] PROGMEM = "0.5.0b";

    /** Arduino is ready for the next command */
    const char READY[] PROGMEM = "\nOK\n";

    const char HASH_FAIL[] PROGMEM = "\nHASH_FAIL\n";

    void printOutputString(const char *targetOutput);

    void sendHandshakeResponse(char *args[]);

    void syncTime(char *args[]);

    void pauseDrive(char **args);

    void unpauseDrive(char **args);

    void deregisterAllEffectors(char **args);

    void clearAllCurves(char **args);

    void registerPinServo(char **args);

    void registerI2CServo(char **args);

    void registerPinStepper(char **args);

    void registerDirStepper(char **args);

    void registerI2CStepper(char **args);

    void registerCurvedEvent(char **args);

    void registerOnOffEvent(char **args);

    void registerTriggerEvent(char **args);

    void registerCustomMotor(char **args);

    void deregisterEffector(char **args);

    void addCurve(char **args);

    void addInstantCurve(char **args);

    void addOnOffCurve(char **args);

    void addTriggerCurve(char **args);

    void manualSync(char **args);

    void clearCurvesForEffector(char **args);

} // namespace BasicCommands

#endif //BOTTANGO_BASICCOMMANDS_H
