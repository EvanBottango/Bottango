#ifndef BOTTANGO_BASICCOMMANDS_H
#define BOTTANGO_BASICCOMMANDS_H

#include "EffectorPool.h"
#include "Errors.h"
#include "System/Time.h"
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

    /** Request to begin the handshake between app and driver */
    inline const char HANDSHAKE_REQUEST[] PROGMEM = "hRQ";

    /** Request to enumerate supported modules on this driver */
    inline const char MODULES_REQUEST[] PROGMEM = "hMOD";

    /** Flag that the sender is ready for the next response */
    inline const char READY_FOR_NEXT_RESPONSE[] PROGMEM = "OK";

    /** Request to establish that the serial port is still open */
    inline const char TIME_SYNC[] PROGMEM = "tSYN";

    /** Stop this controller */
    inline const char STOP[] PROGMEM = "STOP";

    /** Remove all registered effectors */
    inline const char DEREGISTER_ALL_EFFECTORS[] PROGMEM = "xE";

    /** Deregister a Servo type effector with a [0]pin */
    inline const char DEREGISTER_EFFECTOR[] PROGMEM = "xUE";

    /** Remove all active curves */
    inline const char CLEAR_ALL_CURVES[] PROGMEM = "xC";

    /** Clear all curves from effector with the given [0]pin */
    inline const char CLEAR_EFFECTOR_CURVES[] PROGMEM = "xUC";

    /** Update an effectors signal bounds with [0] identifier, [1] min signal, [2] max signal, [3] max signal speed */
    inline const char UPDATE_EFFECTOR_SIGNAL_BOUNDS[] PROGMEM = "upE";

    /** Register a Servo type effector with a [0]pin, [1] minPWM, [2] maxPWM, [3] maxPWMPerSec, [4] startingPWM */
    inline const char REGISTER_PIN_SERVO[] PROGMEM = "rSVPin";

    /** Register a Servo type effector with a [0] i2c address [1]pin, [2] minPWM, [3] maxPWM, [4] maxPWMPerSec, [5] startingPWM */
    inline const char REGISTER_I2C_SERVO[] PROGMEM = "rSVI2C";

    /** Register a Stepper type effector with a [0]pin0, [1]pin1, [2]pin2, [3]pin3, [4]maxCounterClockwiseSteps, [5]maxClockwiseSteps, [6]maxStepsPerSecond */
    inline const char REGISTER_PIN_STEPPER[] PROGMEM = "rSTPin";

    /** Register a Stepper type effector with a [0] step Pin, [1] direction Pin, [2] should clockwise on Low, [3]maxCounterClockwiseSteps, [4]maxClockwiseSteps, [5]maxStepsPerSecond, [6] startingStepOffset */
    inline const char REGISTER_DIR_STEPPER[] PROGMEM = "rSTDir";

    /** Register a Curved Custom Event type effector with a [0] identifier, [1] max movement per second, [2] starting movement, [3] pin */
    inline const char REGISTER_CURVED_EVENT[] PROGMEM = "rECC";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting on, [2] pin */
    inline const char REGISTER_ONOFF_EVENT[] PROGMEM = "rECOnOff";

    /** Register a Trigger Custom Event type effector with a [0] identifier, [1] pin, [2] pin high or low */
    inline const char REGISTER_TRIGGER_EVENT[] PROGMEM = "rECTrig";

    /** Register an On Off Custom Event type effector with a [0] identifier, [1] starting r, [2] starting g, [3] starting b */
    inline const char REGISTER_COLOR_EVENT[] PROGMEM = "rECColor";

    /** Register a custom motor with an [0]identifier, [1] minSignal, [2] maxSignal, [3] maxSignalPerSec, [4] startingSignal */
    inline const char REGISTER_CUSTOM_MOTOR[] PROGMEM = "rMTR";

    /**
     * Command to set a curve on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2] duration of curve, [3] startY, [4] startControlX, [5] startControlY,
     * [6] endY, [7] endControlX, [8] endControlY,
     */
    inline const char SET_CURVE[] PROGMEM = "sC";

    /**
     * Command to set an instant curve on an effector with an
     * [0]identifier, [1] start time relative to last sync, [2] targetMovement,
     */
    inline const char SET_INSTANTCURVE[] PROGMEM = "sCI";

    /**
     * Command to set a on/off on an effector with an
     * [0]identifier, [1] startX relative to last sync, [2]on/off
     */
    inline const char SET_ONOFFCURVE[] PROGMEM = "sCO";

    /**
     * Command to set a trigger on an effector with an
     * [0]identifier, [1] startX relative to last sync,
     */
    inline const char SET_TRIGGERCURVE[] PROGMEM = "sCT";

    /**
     * Command to set a color curve on an effector with an
     * [0]identifier, [1] start time relative to last sync, [2] duration of curve,
     * [3] start Red, [4] start Green, [5] start Blue,
     * [6] end Red, [7] end Green, [8] end Blue,
     */
    inline const char SET_COLOR_CURVE[] PROGMEM = "sCC";

    /**
     * Command to set an instant color curve on an effector with an
     * [0]identifier, [1] final Red, [2] final Green, [3] final Blue,
     */
    inline const char SET_INSTANT_COLOR_CURVE[] PROGMEM = "sCCI";

    /**
     * Command to change motor position in order to sync, without using movement
     * [0]identifier, [1] syncValue
     */
    inline const char STEPPER_SYNC[] PROGMEM = "sycM";
    inline const char STEPPER_SYNC_RESET[] PROGMEM = "rst";
    inline const char STEPPER_SYNC_MANUALHOME[] PROGMEM = "home";
    inline const char STEPPER_SYNC_AUTO_CLOCKWISE[] PROGMEM = "aCW";
    inline const char STEPPER_SYNC_AUTO_COUNTERCLOCKWISE[] PROGMEM = "aCC";

#ifdef RELAY_SUPPORTED
	/**
	 * Command to register a relay controller
	 * [0]identifier, [1] relay connection type, additional tokens connection type dependent
	 */
	const char REGISTER_RELAY[] PROGMEM = "rCtrl";

	/**
	 * Command to deregister a relay controller
	 * [0]identifier
	 */
	const char DEREGISTER_RELAY[] PROGMEM = "xUCtrl";

	/**
	 * Command to deregister all relay controllers
	 * [0]identifier
	 */
	const char DEREGISTER_ALL_RELAY[] PROGMEM = "xCtrl";

	/**
	 * Command to identify a relay command
	 * [0]identifier of relay controller, the rest of tokens are the command to be passed.
	 */
	const char PASS_TO_RELAY[] PROGMEM = "sR";

	/**
	 * Command from bridge to peer to check if it's connectable via a BOOT print
	 */
	const char REQUEST_PEER_BOOT[] PROGMEM = "rBOOT";

	/**
	 * reply From Peer that is now booted and can connect
	 */
	const char REPLY_PEER_BOOT[] PROGMEM = "sBOOT";

	/**
	 * Command to get ESPNOW mac address
	 */
	const char GET_MAC_ADDRESS[] PROGMEM = "rMAC";
#endif

#ifdef ALLOW_SYNC_COMMANDS
    /**
     * Command to identify a syncronized command
     * rest of tokens are the combined command with sync'd syntax
     */
    inline const char SYNC_COMMAND[] PROGMEM = "sSY";
#endif

#ifdef ENABLE_ESP_OTA_UPDATE
    /**
     * Command to update firmware over OTA with ESP32
     * [0] ota message type. s == start, d == data, e == end
     * [1] ota param. s has no param, d is data in 64 byte or less chunk, e is expected checksum of data
     */
    inline const char OTA_UPDATE[] PROGMEM = "ota";
#endif

//#ifdef AUDIO_SD_I2S

    /** Register an audio effector with a [0] identifier, [1] audio file hash */
    //const char REGISTER_AUDIO_EVENT[] PROGMEM = "rAud";
	inline constexpr const char* I2S_RegisterAudioEvent()
	{
		return "rAud";
	}

    /**
     * Command to transfer in an audio file
     * [0] audio bin message type. s == start, d == data, e == end
     * [1] audio bin param. s is audio file name, d is data in 64 byte or less chunk, e is expected checksum of data
     */
    //const char AUDIO_BIN[] PROGMEM = "binA";
//#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
    /**
     * Command to set a configuration option
     * [0] config type
     * [1] config switch option (see below)
     */
    inline const char SET_CONFIG[] PROGMEM = "sCfg";
#endif

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
    /**
     * [1 on sCfg] config switch option, command source
     */
    inline const char SET_CONFIG_COMMAND_SOURCE[] PROGMEM = "CMD"; // set command source sub param
#endif

#ifdef DYNAMIC_STOP_BUTTON_BEHAVIOR
    /**
     * [1 on sCfg] stop button option, 0 is pause, 1 is shutdown
     */
    inline const char SET_CONFIG_STOP_BUTTON[] PROGMEM = "STP_BTN"; // set stop button behavior
#endif

#ifdef RELAY_SUPPORTED
    /**
     * [1 on sCfg] config switch option, relay
     * [2 on relay type 2, peer] bridge mac address
     */
    inline const char SET_CONFIG_RELAY_TYPE[] PROGMEM = "RLY"; // set relay type sub param

    inline const char RELAY_PEER_STOP_TIME[] PROGMEM = "STOP_TIME";   // set command source sub param
	inline const char RELAY_POLL_REQUEST[] PROGMEM = "RLY_POLL";	// Relay poll request
	inline const char RELAY_POLL_RESPONSE[] PROGMEM = "RLY_ACK";	// Relay poll response

#endif

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
	const char DRIVER_VERSION[] PROGMEM = "0.7.1p7a";

	/** Arduino is ready for the next command */
	const char READY[] PROGMEM = "OK\n";

	const char HASH_FAIL[] PROGMEM = "HASH_FAIL\n";

	const char TIMEOUT[] PROGMEM = "TIMEOUT\n";

	const char LOST_PEER[] PROGMEM = "LOST_PEER,";

	/** Request E Stop */
	const char ESTOP[] PROGMEM = "reqStop\n";

	/** Request pause anim */
	const char STOP_PLAY[] PROGMEM = "reqPause\n";

	/** Request start anim */
	const char START_PLAY[] PROGMEM = "reqPlay,";

#ifdef ONLINE_BUTTON_ACTIONS
	const char START_PLAY_BUTTON[] PROGMEM = "reqPlayBtn,";
#endif

	/** Stepper/Custom Motor Auto Sync is Complete */
	const char SYNC_COMPLETE[] PROGMEM = "sycMDone,";

	void sendHandshakeResponse(char* args[], bool sourceIsUsbSerial);

	void startModulesResponse(char* args[]);

	void continueInProgressMultiMessageResponse(char* args[]);

	void stop(char* args[]);

	void syncTime(char* args[]);

	void deregisterAllEffectors(char** args);

	void clearAllCurves(char** args);

	void updateEffectorSignalBounds(char** args);

	void registerPinServo(char** args);

	void registerI2CServo(char** args);

	void registerPinStepper(char** args);

	void registerDirStepper(char** args);

	void registerCurvedEvent(char** args);

	void registerOnOffEvent(char** args);

	void registerTriggerEvent(char** args);

	void registerColorEvent(char** args);

	void registerCustomMotor(char** args);

	void deregisterEffector(char** args);

	void addCurve(char** args);

	void addInstantCurve(char** args);

	void addOnOffCurve(char** args);

	void addTriggerCurve(char** args);

	void addColorCurve(char** args);

	void addInstantColorCurve(char** args);

	void stepperSync(char** args);

	void clearCurvesForEffector(char** args);

#ifdef RELAY_SUPPORTED
	void registerRelayController(char** args);

	void deregisterRelayController(char** args);

	void deregisterAllRelayControllers(char** args);

	void passToRelayController(char** args, byte paramsCount);

	void requestBoot(char** args);

	void requestPoll(char** args);

	void getMACAddress(char** args);
#endif

#ifdef ALLOW_SYNC_COMMANDS
	bool getNextSyncCommand(char syncCmd[], char prefixBuffer[CMD_PREFIX_SIZE], char outputCommand[MAX_COMMAND_LENGTH]);
	void beginGetNextSyncCommand(char* syncCmd, char prefixBuffer[CMD_PREFIX_SIZE]);
	void executeSyncronizedCommands(char* syncCmd, bool secondary);
#endif
#ifdef ENABLE_ESP_OTA_UPDATE
	void processOTA(char** args);
#endif

#ifdef AUDIO_SD_I2S
	//void processAudioBinary(char** args);
	void registerAudioEvent(char** args);
#endif

#if defined(AUDIO_SD_I2S) || defined(ENABLE_ESP_OTA_UPDATE)
#define BINARY_FLAG_START 's'
#define BINARY_FLAG_DATA 'd'
#define BINARY_FLAG_END 'e'
#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
	void setConfiguration(char** args);
#endif

	void reboot(bool forceSendReady);

} // namespace BasicCommands

#endif // BOTTANGO_BASICCOMMANDS_H