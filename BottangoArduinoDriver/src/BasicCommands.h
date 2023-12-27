#ifndef BOTTANGO_BASICCOMMANDS_H
#define BOTTANGO_BASICCOMMANDS_H

#include "Log.h"
#include "EffectorPool.h"
#include "Errors.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
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
    const char TIME_SYNC[] PROGMEM = "tSYN";

    /** Stop this controller */
    const char STOP[] PROGMEM = "STOP";

    /** Remove all registered effectors */
    const char DEREGISTER_ALL_EFFECTORS[] PROGMEM = "xE";

    /** Deregister a Servo type effector with a [0]pin */
    const char DEREGISTER_EFFECTOR[] PROGMEM = "xUE";

    /** Remove all active curves */
    const char CLEAR_ALL_CURVES[] PROGMEM = "xC";

    /** Clear all curves from effector with the given [0]pin */
    const char CLEAR_EFFECTOR_CURVES[] PROGMEM = "xUC";

    /** Update an effectors signal bounds with [0] identifier, [1] min signal, [2] max signal, [3] max signal speed */
    const char UPDATE_EFFECTOR_SIGNAL_BOUNDS[] PROGMEM = "upE";

    /** Register a Servo type effector with a [0]pin, [1] minPWM, [2] maxPWM, [3] maxPWMPerSec, [4] startingPWM */
    const char REGISTER_PIN_SERVO[] PROGMEM = "rSVPin";

    /** Register a Servo type effector with a [0] i2c address [1]pin, [2] minPWM, [3] maxPWM, [4] maxPWMPerSec, [5] startingPWM */
    const char REGISTER_I2C_SERVO[] PROGMEM = "rSVI2C";

    /** Register a Stepper type effector with a [0]pin0, [1]pin1, [2]pin2, [3]pin3, [4]maxCounterClockwiseSteps, [5]maxClockwiseSteps, [6]maxStepsPerSecond */
    const char REGISTER_PIN_STEPPER[] PROGMEM = "rSTPin";

    /** Register a Stepper type effector with a [0] step Pin, [1] direction Pin, [2] should clockwise on Low, [3]maxCounterClockwiseSteps, [4]maxClockwiseSteps, [5]maxStepsPerSecond, [6] startingStepOffset */
    const char REGISTER_DIR_STEPPER[] PROGMEM = "rSTDir";

    /** Register a Curved Custom Event type effector with a [0] identifier, [1] max movement per second, [2] starting movement */
    const char REGISTER_CURVED_EVENT[] PROGMEM = "rECC";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting on */
    const char REGISTER_ONOFF_EVENT[] PROGMEM = "rECOnOff";

    /** Register an On Off Custom Event type effector with a [0] identifier */
    const char REGISTER_TRIGGER_EVENT[] PROGMEM = "rECTrig";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting r, [2] starting g, [3] starting b */
    const char REGISTER_COLOR_EVENT[] PROGMEM = "rECColor";

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
     * Command to set a color curve on an effector with an
     * [0]identifier, [1] start time relative to last sync, [2] duration of curve,
     * [3] start Red, [4] start Green, [5] start Blue,
     * [6] end Red, [7] end Green, [8] end Blue,
     */
    const char SET_COLOR_CURVE[] PROGMEM = "sCC";

    /**
     * Command to set an instant color curve on an effector with an
     * [0]identifier, [1] final Red, [2] final Green, [3] final Blue,
     */
    const char SET_INSTANT_COLOR_CURVE[] PROGMEM = "sCCI";

    /**
     * Command to change motor position in order to sync, without using movement
     * [0]identifier, [1] syncValue
     */
    const char STEPPER_SYNC[] PROGMEM = "sycM";

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
    const char DRIVER_VERSION[] PROGMEM = "0.6.4a";

    /** Arduino is ready for the next command */
    const char READY[] PROGMEM = "\nOK\n";

    const char HASH_FAIL[] PROGMEM = "\nHASH_FAIL\n";

    const char TIMEOUT[] PROGMEM = "\nTIMEOUT\n";

    void printOutputString(const char *targetOutput);

    void sendHandshakeResponse(char *args[]);

    void stop(char *args[]);

    void syncTime(char *args[]);

    void deregisterAllEffectors(char **args);

    void clearAllCurves(char **args);

    void updateEffectorSignalBounds(char **args);

    void registerPinServo(char **args);

    void registerI2CServo(char **args);

    void registerPinStepper(char **args);

    void registerDirStepper(char **args);

    void registerCurvedEvent(char **args);

    void registerOnOffEvent(char **args);

    void registerTriggerEvent(char **args);

    void registerColorEvent(char **args);

    void registerCustomMotor(char **args);

    void deregisterEffector(char **args);

    void addCurve(char **args);

    void addInstantCurve(char **args);

    void addOnOffCurve(char **args);

    void addTriggerCurve(char **args);

    void addColorCurve(char **args);

    void addInstantColorCurve(char **args);

    void stepperSync(char **args);

    void clearCurvesForEffector(char **args);

} // namespace BasicCommands

#endif // BOTTANGO_BASICCOMMANDS_H
