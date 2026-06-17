
#include "BasicCommands.h"
#include "../../BottangoArduinoModules.h"

#include "../Effector/PinServoEffector.h"
#include "../Effector/PinStepperEffector.h"
#include "../Effector/I2CServoEffector.h"
#include "../Effector/StepDirStepperEffector.h"
#include "../Effector/CurvedCustomEvent.h"
#include "../Effector/ColorCustomEvent.h"
#include "../Effector/OnOffCustomEvent.h"
#include "../Effector/OnOffCurve.h"
#include "../Effector/TriggerCustomEvent.h"
#include "../Effector/TriggerCurve.h"
#include "../Effector/ColorCurve.h"
#include "../Effector/CustomMotorEffector.h"
#include "../Communication/Outgoing.h"

#ifdef AUDIO_SD_I2S
#include "../Effector/I2SAudioEffector.h"
#endif

#ifdef RELAY_SUPPORTED
#include "../Util/UDIDHelper.h"
#include "../Communication/Relay Coms/RelayChild.h"
#include "../Communication/Relay Coms/MACResponder.h"
#include "../Communication/Relay Coms/IRelayComms.h"
#endif

#ifdef ENABLE_STATUS_LIGHTS
#include "../Services/StatusLights.h"
#endif

#ifdef ENABLE_ESP_OTA_UPDATE
#include "../OTA/OTAUpdateUtil.h"
#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
#include "../Services/PersistentConfigUtil.h"
#endif

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR)
#include <avr/wdt.h>
#endif

namespace BasicCommands
{
	// input [0] command, [1] hash code
	// output [0] command, [1] driver version, [2] repeat back hash code
	void sendHandshakeResponse(char* args[], bool secondary)
	{
		bool offlinePlayback = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
		if (PersistentConfigUtil::getUseExportedCommandStream())
		{
			offlinePlayback = true;
		}
#else
		offlinePlayback = true;
#endif
#endif

		if (!secondary)
		{
			BottangoCore::initialized = true;
			BottangoCore::handshake = true;

			if (!offlinePlayback)
			{
				deregisterAllEffectors(NULL);
			}
		}

		char* code = args[1];

		// command name
		Outgoing::printOutputStringPROGMEM(BasicCommands::HANDSHAKE);
		Outgoing::printOutputStringFlash(F(","));

		// driver version
		Outgoing::printOutputStringPROGMEM(BasicCommands::DRIVER_VERSION);
		Outgoing::printOutputStringFlash(F(","));

		// repeat back hash code
		Outgoing::printOutputStringMem(code);

		// complete
		Outgoing::printLine();

		Serial.flush();

		if (!secondary)
		{
#ifdef ENABLE_STATUS_LIGHTS
#if defined(RELAY_SUPPORTED)
			if (!offlinePlayback)
			{
				StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_HAS_CONNECTION);
				StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_PULSE);
			}

#elif defined(USE_USB_SERIAL)
			StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, STATUS_COLOR_HAS_CONNECTION);
			StatusLights::setLightMode(CONNECTION_STATUS_LIGHT, StatusLights::LightMode::MODE_PULSE);
#endif
#endif

#if defined(RELAY_SUPPORTED)
			if (BottangoCore::isRelayPeer)
			{
				BottangoCore::lastPollTimeAsPeer = millis();
			}
#endif

			Callbacks::onThisControllerStarted();
		}
	}

	// initialize modules response
	void startModulesResponse(char* args[])
	{
		// this is the only modules response allowed to execute in offline mode
		BottangoCore::replaceActiveOutgoingMultimessage(new ModulesResponder());
	}

	void continueInProgressMultiMessageResponse(char* args[])
	{
		if (BottangoCore::activeOutgoingMultimessage == nullptr)
		{
			// should have an active...
			return;
		}
		BottangoCore::activeOutgoingMultimessage->setRecievedContinue();
	}

	void stop(char* args[])
	{
		BottangoCore::stop(true);
	}

	void syncTime(char* args[])
	{
#ifdef RELAY_SUPPORTED
		if (strcmp_P(args[1], BasicCommands::RELAY_PEER_STOP_TIME) == 0)
		{
			Time::stopTime();
			return;
		}
#endif
		Time::syncTime(atol(args[1]));
	}

	void deregisterAllEffectors(char** args)
	{
		BottangoCore::effectorPool.deregisterAll();
	}

	void clearAllCurves(char** args)
	{
		BottangoCore::effectorPool.clearAllCurves();
	}

	void updateEffectorSignalBounds(char** args)
	{
		char* identifier = args[1];
		int minSignal = atoi(args[2]);
		int maxSignal = atoi(args[3]);
		int signalSpeed = atoi(args[4]);

		BottangoCore::effectorPool.updateEffectorSignalBounds(identifier, minSignal, maxSignal, signalSpeed);
	}

	void registerPinServo(char** args)
	{

		byte pinId = atoi(args[1]);
		int minPWM = atoi(args[2]);
		int maxPWM = atoi(args[3]);
		int maxPWMSec = atoi(args[4]);
		int startPWM = atoi(args[5]);

		PinServoEffector* newEffector = new PinServoEffector(pinId, minPWM, maxPWM, maxPWMSec, startPWM);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerI2CServo(char** args)
	{

#ifndef USE_ADAFRUIT_PWM_LIBRARY
		Error::reportError_MissingLibrary("i2c servo");
		return;
#endif

		byte address = atoi(args[1]);
		byte pinId = atoi(args[2]);
		int minPWM = atoi(args[3]);
		int maxPWM = atoi(args[4]);
		int maxPWMSec = atoi(args[5]);
		int startPWM = atoi(args[6]);

		I2CServoEffector* newEffector = new I2CServoEffector(address, pinId, minPWM, maxPWM, maxPWMSec, startPWM);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerPinStepper(char** args)
	{

		byte pin0 = atoi(args[1]);
		byte pin1 = atoi(args[2]);
		byte pin2 = atoi(args[3]);
		byte pin3 = atoi(args[4]);

		int maxCounterClockwiseSteps = atoi(args[5]);
		int maxClockwiseSteps = atoi(args[6]);
		int maxStepsPerSecond = atoi(args[7]);
		int startingStepOffset = atoi(args[8]);

		PinStepperEffector* newEffector = new PinStepperEffector(pin0, pin1, pin2, pin3, maxCounterClockwiseSteps, maxClockwiseSteps, maxStepsPerSecond, startingStepOffset);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerDirStepper(char** args)
	{

		byte stepPin = atoi(args[1]);
		byte dirPin = atoi(args[2]);

		bool clockwiseIsLow = atoi(args[3]) != 0;

		int maxCounterClockwiseSteps = atoi(args[4]);
		int maxClockwiseSteps = atoi(args[5]);
		int maxStepsPerSecond = atoi(args[6]);
		int startingStepOffset = atoi(args[7]);

		StepDirStepperEffector* newEffector = new StepDirStepperEffector(stepPin, dirPin, clockwiseIsLow, maxCounterClockwiseSteps, maxClockwiseSteps, maxStepsPerSecond, startingStepOffset);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerCurvedEvent(char** args)
	{
		char* identifier = args[1];
		float maxSpeed = atof(args[2]);
		float startingMovement = atof(args[3]);
		byte pin = atoi(args[4]);

		CurvedCustomEvent* newEffector = new CurvedCustomEvent(identifier, maxSpeed, startingMovement, pin);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerOnOffEvent(char** args)
	{
		char* identifier = args[1];
		bool startOn = atoi(args[2]) != 0;
		byte pin = atoi(args[3]);

		OnOffCustomEvent* newEffector = new OnOffCustomEvent(identifier, startOn, pin);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerTriggerEvent(char** args)
	{
		char* identifier = args[1];
		byte pin = atoi(args[2]);
		bool fireIsHigh = atoi(args[3]) != 0;

		TriggerCustomEvent* newEffector = new TriggerCustomEvent(identifier, pin, fireIsHigh);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

#ifdef AUDIO_SD_I2S
	void registerAudioEvent(char** args)
	{
		char* identifier = args[1];
		char* hash = args[2];

		I2SAudioEffector* newEffector = new I2SAudioEffector(identifier, hash);
		BottangoCore::effectorPool.addEffector(newEffector);
	}
#endif

	void registerColorEvent(char** args)
	{
		char* identifier = args[1];

		byte r = atoi(args[2]);
		byte g = atoi(args[3]);
		byte b = atoi(args[4]);

		ColorCustomEvent* newEffector = new ColorCustomEvent(identifier, r, g, b);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void registerCustomMotor(char** args)
	{
		char* identifier = args[1];
		int minSignal = atoi(args[2]);
		int maxSignal = atoi(args[3]);
		int maxSignalSec = atoi(args[4]);
		int startSignal = atoi(args[5]);

		CustomMotorEffector* newEffector = new CustomMotorEffector(identifier, minSignal, maxSignal, maxSignalSec, startSignal);
		BottangoCore::effectorPool.addEffector(newEffector);
	}

	void deregisterEffector(char** args)
	{
		char* identifier = args[1];

		BottangoCore::effectorPool.removeEffector(identifier);
	}

	/**
	 * Command to set a curve on an effector with an
	 * [0]identifier, [1] startX relative to last sync, [2] duration of curve, [3] startY, [4] startControlX, [5] startControlY,
	 * [6] endY, [7] endControlX, [8] endControlY,
	 */

	void addCurve(char** args)
	{
		char* identifier = args[1];

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
	void addInstantCurve(char** args)
	{
		char* identifier = args[1];

		// end movement is int 0 - COMPRESSED_SIGNAL_MAX
		int endMovement = atoi(args[2]);

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
	void addOnOffCurve(char** args)
	{
		char* identifier = args[1];

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

		BottangoCore::effectorPool.addCurveToEffector(identifier, new OnOffCurve(startTime, on));
	}

	/**
	 * Command to set a on/off on an effector with an
	 * [0]identifier, [1] startX relative to last sync, [2]on/off
	 */
	void addTriggerCurve(char** args)
	{
		char* identifier = args[1];

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
	}

	void addColorCurve(char** args)
	{
		char* identifier = args[1];

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

		BottangoCore::effectorPool.addCurveToEffector(identifier, new ColorCurve(startX, duration, startR, startG, startB, endR, endG, endB));
	}

	/**
	 * Command to set an instant and immediate curve on an effector with an
	 * [1]identifier, [2] targetR, [3] targetG, [4] targetB
	 */
	void addInstantColorCurve(char** args)
	{
		char* identifier = args[1];

		int r = atoi(args[2]);
		int g = atoi(args[3]);
		int b = atoi(args[4]);

		BottangoCore::effectorPool.addCurveToEffector(identifier, new ColorCurve(Time::getCurrentTimeInMs(), 0, r, g, b, r, g, b));
	}

	/**
	 * Command to sync a stepper type effector
	 * [1]identifier, [2] targetSync (0 - 100 and 0 - -100 are manual sync) ( any number > 100 and <-100 are auto sync in the given direction)
	 */
	void stepperSync(char** args)
	{
		char* identifier = args[1];

		if (strncmp(args[2], BasicCommands::STEPPER_SYNC_MANUALHOME, 3) == 0)
		{
			BottangoCore::effectorPool.homeEffector(identifier);
		}
		else if (strncmp(args[2], BasicCommands::STEPPER_SYNC_RESET, 4) == 0)
		{
			BottangoCore::effectorPool.resetHomeEffector(identifier);
		}
		else if (strncmp(args[2], BasicCommands::STEPPER_SYNC_AUTO_CLOCKWISE, 4) == 0)
		{
			BottangoCore::effectorPool.autoSyncEffector(identifier, 1);
		}
		else if (strncmp(args[2], BasicCommands::STEPPER_SYNC_AUTO_COUNTERCLOCKWISE, 4) == 0)
		{
			BottangoCore::effectorPool.autoSyncEffector(identifier, -1);
		}
		else
		{
			int syncVal = atoi(args[2]);
			BottangoCore::effectorPool.syncEffector(identifier, syncVal);
		}
	}

	void clearCurvesForEffector(char** args)
	{
		char* identifier = args[1];

		BottangoCore::effectorPool.clearCurvesForEffector(identifier);
	}

#ifdef RELAY_SUPPORTED
	void registerRelayController(char** args)
	{
		if (!BottangoCore::isRelayBridge)
		{
			Outgoing::printOutputStringFlash(F("Aborting register relay, Not bridge"));
			Outgoing::printLine();
			return;
		}

		char* macAddress = args[1];

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			Outgoing::printOutputStringFlash(F("Reg Relay with Mac: "));
			Outgoing::printOutputStringMem(macAddress);
			Outgoing::printLine();
		}
#endif

		RelayChild* newRelay = new RelayChild(macAddress);
		BottangoCore::relayPool->addRelay(newRelay);
		int relayId = BottangoCore::relayPool->getIdForRelay(newRelay);
		BottangoCore::relayPool->setRelayIdToReport(relayId);

		if (BottangoCore::isOffline())
		{
			return;
		}

		BottangoCore::replaceActiveOutgoingMultimessage(BottangoCore::relayPool);
	}

	void deregisterRelayController(char** args)
	{
		int id = atoi(args[1]);
		BottangoCore::relayPool->removeRelay(id);
	}

	void deregisterAllRelayControllers(char** args)
	{
		BottangoCore::relayPool->deregisterAll();
	}

	void passToRelayController(char** args, byte paramsCount)
	{
		int id = atoi(args[1]);
		BottangoCore::relayPool->passThroughCommandToRelay(id, args, paramsCount);
	}

	void requestBoot(char** args)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Peer recieved boot request from bridge"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif
		Outgoing::printOutputStringPROGMEM(BasicCommands::REPLY_PEER_BOOT);
		Outgoing::printLine();
	}

	void requestPoll(char** args)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Peer recieved poll request from bridge"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif
		Outgoing::printOutputStringPROGMEM(BasicCommands::RELAY_POLL_RESPONSE);
		Outgoing::printLine();
		BottangoCore::lastPollTimeAsPeer = millis();
	}

	// initialize mac address response
	void getMACAddress(char* args[])
	{
		if (BottangoCore::isOffline())
		{
			return;
		}

		BottangoCore::replaceActiveOutgoingMultimessage(new MACResponder());
	}
#endif

#ifdef ALLOW_SYNC_COMMANDS
	// Finds the next ';' in inputString, severs it, returns it in output
	// as one full command (prefix+data), then slides the remainder forward.
	// Returns true if it consumed a command, false when no ';' remains.
	bool getNextSyncCommand(char syncCmd[], char prefixBuffer[CMD_PREFIX_SIZE], char outputCommand[MAX_COMMAND_LENGTH])
	{
		// empty
		// or starts with newline (export end of command)
		// or starts with , (live control ends with ,hXYZ hash)
		if (syncCmd[0] == '\0' || syncCmd[0] == '\n' || syncCmd[0] == ',')
		{
			return false;
		}

		// Update prefix if this chunk starts with '*'
		if (syncCmd[0] == '*')
		{
			char* commaPtr = strchr(syncCmd + 1, ',');
			int newPrefixLen = int(commaPtr - (syncCmd + 1) + 1);
			memcpy(prefixBuffer, syncCmd + 1, newPrefixLen);
			prefixBuffer[newPrefixLen] = '\0';

			// Remove the "*prefix," from buffer
			char* remainder = commaPtr + 1;
			memmove(syncCmd, remainder, strlen(remainder) + 1);

			// Recurse to fetch the actual command next
			return getNextSyncCommand(syncCmd, prefixBuffer, outputCommand);
		}

		// Find the next ';' to isolate a raw chunk
		char* semicolonPtr = strchr(syncCmd, ';');
		size_t rawChunkLength;
		char* nextStart;

		if (semicolonPtr)
		{
			rawChunkLength = semicolonPtr - syncCmd;
			nextStart = semicolonPtr + 1;
		}
		else
		{
			rawChunkLength = strlen(syncCmd);
			nextStart = syncCmd + rawChunkLength;
		}

		// Skip over existing prefix in the chunk
		int prefixLen = strlen(prefixBuffer);
		char* dataStart = syncCmd;
		size_t dataLength = rawChunkLength;

		if (rawChunkLength >= (size_t)prefixLen &&
			strncmp(syncCmd, prefixBuffer, prefixLen) == 0)
		{
			dataStart = syncCmd + prefixLen;
			dataLength = rawChunkLength - prefixLen;
		}

		// Build outputCommand = prefix + dataStart
		size_t copyLength = dataLength;
		if ((size_t)prefixLen + copyLength >= MAX_COMMAND_LENGTH)
		{
			copyLength = MAX_COMMAND_LENGTH - prefixLen - 1;
		}

		memcpy(outputCommand, prefixBuffer, prefixLen);
		memcpy(outputCommand + prefixLen, dataStart, copyLength);
		outputCommand[prefixLen + copyLength] = '\0';

		// Remove the processed chunk (and its ';') from buffer
		memmove(syncCmd, nextStart, strlen(nextStart) + 1);

		return true;
	}

	void beginGetNextSyncCommand(char* syncCmd, char prefixBuffer[CMD_PREFIX_SIZE])
	{
		// 1) Strip up to and including the first comma
		char* commaPtr = strchr(syncCmd, ',');
		memmove(syncCmd, commaPtr + 1, strlen(commaPtr + 1) + 1);

		// 2) Seed prefixBuffer with the first token
		commaPtr = strchr(syncCmd, ',');
		int seedLength = int(commaPtr - syncCmd + 1);
		memcpy(prefixBuffer, syncCmd, seedLength);
		prefixBuffer[seedLength] = '\0';
	}

	void executeSyncronizedCommands(char* syncCmd, bool secondary)
	{
		char prefixBuffer[CMD_PREFIX_SIZE] = { 0 };
		beginGetNextSyncCommand(syncCmd, prefixBuffer);

		// Loop and execute each command
		char singleCommand[MAX_COMMAND_LENGTH] = { 0 };
		while (getNextSyncCommand(syncCmd, prefixBuffer, singleCommand))
		{
			BottangoCore::executeCommand(singleCommand, secondary);
		}
	}
#endif

	void reboot(bool forceSendReady)
	{
		if (forceSendReady)
		{
			Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
			// cover all bases, send secondary ok as well, if the config command came over serial
#ifdef RELAY_SUPPORTED
			if (!Outgoing::secondaryPeerOutgoing)
			{
				Outgoing::setSecondaryPeerOutgoing(true);
				Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
				Outgoing::flush();
				Outgoing::setSecondaryPeerOutgoing(false);
			}
#endif
		}

		Outgoing::flush();
		delay(150);

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
		ESP.restart();
		for (;;)
		{
		}
#elif defined(TEENSYDUINO)
		SCB_AIRCR = 0x05FA0004;
		for (;;)
		{
		}
#elif defined(ARDUINO_ARCH_AVR)
		wdt_enable(WDTO_15MS);
		for (;;)
		{
		}
#elif defined(ARDUINO_ARCH_MEGAAVR)
#if defined(WDT_PERIOD_8CLK_gc)
		wdt_enable(WDT_PERIOD_8CLK_gc);
#elif defined(WDT_PERIOD_8KCLK_gc)
		wdt_enable(WDT_PERIOD_8KCLK_gc);
#else
#warning "megaAVR watchdog reset constant not found"
#endif
		for (;;)
		{
		}
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_RENESAS) || (defined(__arm__) && defined(NVIC_SystemReset))
		NVIC_SystemReset();
		for (;;)
		{
		}
#else
#warning "reboot not supported yet for your platform. May see unexpected behavior"
#endif
	}

#ifdef ENABLE_ESP_OTA_UPDATE
	void processOTA(char** args)
	{
		char otaMessageType = args[1][0];

		if (otaMessageType == BINARY_FLAG_START)
		{
			OTAUpdateUtil::beginOTA();
		}
		else if (otaMessageType == BINARY_FLAG_DATA)
		{
			OTAUpdateUtil::recvOTAData(args[2]);
		}
		else if (otaMessageType == BINARY_FLAG_END)
		{
			OTAUpdateUtil::finishOTA(args[2]);
		}
	}
#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
	void setConfiguration(char** args)
	{
#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH)
		// set command source
		if (strcmp_P(args[1], BasicCommands::SET_CONFIG_COMMAND_SOURCE) == 0)
		{
			int setValue = atoi(args[2]);
			if (setValue != 2 && setValue != 3)
			{
				return;
			}
			bool shouldUseExported = setValue == 3;
			bool currentUseExported = PersistentConfigUtil::getUseExportedCommandStream();
			if (currentUseExported != shouldUseExported)
			{
				PersistentConfigUtil::setUseExportedCommandStream(shouldUseExported);
				reboot(true);
			}
			return;
		}
#endif

#if defined(DYNAMIC_STOP_BUTTON_BEHAVIOR)
		// set command source
		if (strcmp_P(args[1], BasicCommands::SET_CONFIG_STOP_BUTTON) == 0)
		{
			int setValue = atoi(args[2]);
			PersistentConfigUtil::setStopIsShutdown((bool)setValue);
			return;
		}
#endif

#if defined(RELAY_SUPPORTED)
		if (strcmp_P(args[1], BasicCommands::SET_CONFIG_RELAY_TYPE) == 0)
		{
			int setValue = atoi(args[2]);
			if (setValue > VALUE_RELAY_STATE_PEER)
			{
				setValue = 0;
			}

			int currentState = PersistentConfigUtil::getRelayState();

			// pass bridge address when setting to peer
			if (setValue == VALUE_RELAY_STATE_PEER)
			{
				// may just be updating bridge address
				uint8_t newMac[6];
				uint8_t currentMac[6];
				(void)UDIDHelper::convertCStrToMAC(args[3], newMac);
				bool gotBridgeMac = PersistentConfigUtil::getBridgeMacAddress(currentMac);

				bool newMatchesCurrent = false;
				if (gotBridgeMac)
				{
					newMatchesCurrent = true;
					for (int i = 0; i < 6; i++)
					{
						if (newMac[i] != currentMac[i])
						{
							newMatchesCurrent = false;
							break;
						}
					}
				}

				if (!newMatchesCurrent)
				{
					PersistentConfigUtil::storeBridgeMacAddress(newMac);
				}
				// or may be switching to peer, or both!
				if (setValue != currentState)
				{
					PersistentConfigUtil::setRelayState(setValue);
				}
				reboot(true);
			}
			else
			{
				if (setValue != currentState)
				{
					PersistentConfigUtil::setRelayState(setValue);
					reboot(true);
				}
			}
			return;
		}
#endif
	}
#endif

} // namespace BasicCommands
