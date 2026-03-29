
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
#include "Modules/OutgoingSerial.h"
#include "../BottangoArduinoModules.h"
#include "System/SystemStatus.h"
#include "Module Handling/ModuleMaster.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioEffector.h"
#include "AudioBinaryUtil.h"
#endif

#ifdef RELAY_SUPPORTED
#include "Modules/OutgoingRelay.h"
#include "Modules/RelayComs/RelayChild.h"
#include "Modules/RelayComs/UDIDHelper.h"
#include "Modules/RelayComs/Relay.h"
#include "MACResponder.h"
#endif

#ifdef ENABLE_STATUS_LIGHTS
#include "Modules/StatusLightsModule.h"
#endif

#ifdef ENABLE_ESP_OTA_UPDATE
#include "OTAUpdateUtil.h"
#endif

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
#include "PersistentConfigUtil.h"
#endif

namespace BasicCommands
{
	// input [0] command, [1] hash code
	// output [0] command, [1] driver version, [2] repeat back hash code
	void sendHandshakeResponse(char* args[], bool sourceIsUsbSerial)
	{
		bool offlinePlayback = false;
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
		offlinePlayback = PersistentConfigUtil::getUseExportedCommandStream();
#else 
		offlinePlayback = true;
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#endif // USE_CODE_COMMAND_STREAM || USE_SD_CARD_COMMAND_STREAM

#ifdef RELAY_SUPPORTED
		if (!sourceIsUsbSerial)
		{
			SystemStatus::systemStatus.initialized = true;
			SystemStatus::systemStatus.handshake = true;

			if (!offlinePlayback)
			{
				deregisterAllEffectors(NULL);
			}
		}
#endif // RELAY_SUPPORTED

		// command name
		Outgoing::printOutputStringPROGMEM(BasicCommands::HANDSHAKE);
		Outgoing::printOutputStringFlash(F(","));

		// driver version
		Outgoing::printOutputStringPROGMEM(BasicCommands::DRIVER_VERSION);
		Outgoing::printOutputStringFlash(F(","));

		// repeat back hash code
		Outgoing::printOutputStringMem(args[1]);

		// complete
		Outgoing::printLine();

		Serial.flush();

		// ToDo: Irgendwas stimmt hier nicht mit dem Status der gesetzt wird? Der Compile-Guard für die LEDs ist auch nicht mehr nötig
#if defined(RELAY_SUPPORTED)
		if (!sourceIsUsbSerial)
		{
#ifdef ENABLE_STATUS_LIGHTS
			if (!offlinePlayback)
			{
				SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Has_Connection;
			}
#elif defined(USE_USB_SERIAL)
			SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Has_Connection;
#endif // ENABLE_STATUS_LIGHTS

			Relay* relay = BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs);
			if (relay->getRole() == Relay::RelayRole::Peer)
			{
				//relay->setLastHeartbeatTime(millis());
				BottangoCore::lastPollTimeAsPeer = millis();
			}
		}
#endif // RELAY_SUPPORTED
		Callbacks::onThisControllerStarted();
	}

	// initialize modules response
	void startModulesResponse(char* args[])
	{
		if (BottangoCore::activeOutgoingMultimessage != nullptr)
		{
			// shouldn't have an active...
			BottangoCore::activeOutgoingMultimessage->cleanUpMultiMessage();
			BottangoCore::activeOutgoingMultimessage = nullptr;
		}
		BottangoCore::activeOutgoingMultimessage = new ModulesResponder();

		BottangoCore::activeOutgoingMultimessage->initializeMultiMessage();
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
		BottangoCore::stop();
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

		BottangoCore::effectorPool.addEffector<I2CServoEffector>(address, pinId, minPWM, maxPWM, maxPWMSec, startPWM);
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

		BottangoCore::effectorPool.addEffector<I2SAudioEffector>(identifier, hash, static_cast<IAudioPlayback*>(InterfaceRegistry::get(Modules::AudioI2S)));

		//I2SAudioEffector *newEffector = new I2SAudioEffector(identifier, hash, static_cast<IAudioPlayback*>(InterfaceRegistry::get(Modules::AudioI2S)));
		//BottangoCore::effectorPool.addEffector(newEffector);
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
		// ToDo: We need to do something about this special case with the offset. This adds #ifdef clutter we want (and need) to avoid.
		// But since this is a bit deeper down in the general workings of everything, I'm not sure how to best refactor this right now.
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
	void registerPeer(char** args)
	{
		Relay* relay = BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs);
		if (relay->getRole() != Relay::RelayRole::Bridge)
		{
			OutgoingSerial::printOutputStringFlash(F("Aborting register peer, Not bridge"));
			OutgoingSerial::printLine();
			return;
		}

		char* macAddress = args[1];

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			OutgoingSerial::printOutputStringFlash(F("Reg Relay with Mac: "));
			OutgoingSerial::printOutputStringMem(macAddress);
			OutgoingSerial::printLine();
		}
#endif // RELAY_LOGGING

		RelayChild* newPeer = new RelayChild(macAddress);
		relay->getPeerPool()->addPeer(newPeer);

		int relayId = relay->getPeerPool()->getIdForPeer(newPeer);
		relay->getPeerPool()->setPeerIdToReport(relayId);

		if (BottangoCore::activeOutgoingMultimessage != nullptr)
		{
			// shouldn't have an active...
			BottangoCore::activeOutgoingMultimessage->cleanUpMultiMessage();
			BottangoCore::activeOutgoingMultimessage = nullptr;
		}
		BottangoCore::activeOutgoingMultimessage = relay->getPeerPool();

		// ToDo: Was passiert hier genau mit secondaryPeerOutgoing? Warum muss das hier jetzt über die Serielle Schnittstelle geschickt werden?
		// Wann muss das über "Relay" geschickt werden? Wird das hier nur gedreht, weil wir als Relay default Outgoing = Relay haben?
		// registerPeer() sollte immer nur von Bottango direkt angesprochen werden, oder über Offline (SD-Karte...) setup
/*#ifdef RELAY_SUPPORTED
		if (Outgoing::secondaryPeerOutgoing)
		{
			BottangoCore::activeOutgoingMultimessage->setSecondary();
		}
#endif // RELAY_SUPPORTED*/
		BottangoCore::activeOutgoingMultimessage->initializeMultiMessage();
	}

	void deregisterPeer(char** args)
	{
		int id = atoi(args[1]);
		BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->removePeer(id);
	}

	void deregisterAllPeers(char** args)
	{
		BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->deregisterAll();
	}

	void passToPeer(char** args)
	{
		int id = atoi(args[1]);
		BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->passThroughCommandToPeer(id, args);
	}

	void requestBoot(char** args)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Peer recieved boot request from bridge"));
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		Outgoing::printOutputStringPROGMEM(BasicCommands::REPLY_PEER_BOOT);
		Outgoing::printLine();
	}

	void requestPoll(char** args)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Peer recieved poll request from bridge"));
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		Outgoing::printOutputStringPROGMEM(BasicCommands::RELAY_POLL_RESPONSE);
		Outgoing::printLine();
		BottangoCore::lastPollTimeAsPeer = millis();
		//BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->setLastHeartbeatTime(millis());
	}

	// initialize mac address response
	void getMACAddress(char** args)
	{
		if (BottangoCore::activeOutgoingMultimessage != nullptr)
		{
			// shouldn't have an active...
			BottangoCore::activeOutgoingMultimessage->cleanUpMultiMessage();
			BottangoCore::activeOutgoingMultimessage = nullptr;
		}
		BottangoCore::activeOutgoingMultimessage = new MACResponder();
		if (Outgoing::secondaryPeerOutgoing)
		{
			BottangoCore::activeOutgoingMultimessage->setSecondary();
		}
		BottangoCore::activeOutgoingMultimessage->initializeMultiMessage();
	}
#endif // RELAY_SUPPORTED

	void reboot(bool forceSendReady)
	{
		if (forceSendReady)
		{
			// cover all bases, send OK to all outputs
			OutgoingSerial::printOutputStringPROGMEM(BasicCommands::READY);
			OutgoingSerial::flush();
#ifdef RELAY_SUPPORTED
			OutgoingRelay::printOutputStringPROGMEM(BasicCommands::READY);
			OutgoingRelay::flush();
#endif // RELAY_SUPPORTED
		}

#ifdef ESP32
		// this feels risky?
		delay(150);
		ESP.restart();
#elif defined(TEENSYDUINO)
		SCB_AIRCR = 0x05FA0004;
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
#endif // ENABLE_ESP_OTA_UPDATE

#ifdef AUDIO_SD_I2S
	// ToDo: AudioBinaryUtil currently disabled (unfinished feature)
		//void processAudioBinary(char** args)
		//{
			/*char binaryMessageType = args[1][0];

			if (binaryMessageType == BINARY_FLAG_START)
			{
				char *identifier = args[2];
				bool isHash = atoi(args[3]) != 0;
				AudioBinaryUtil::beginAudioBinary(identifier, isHash);
			}
			else if (binaryMessageType == BINARY_FLAG_DATA)
			{
				AudioBinaryUtil::recvAudioBinaryData(args[2]);
			}
			else if (binaryMessageType == BINARY_FLAG_END)
			{
				AudioBinaryUtil::finishAudioBinary(args[2]);
			}*/
			//}
#endif // AUDIO_SD_I2S

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
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH

#if defined(DYNAMIC_STOP_BUTTON_BEHAVIOR)
		// set command source
		if (strcmp_P(args[1], BasicCommands::SET_CONFIG_STOP_BUTTON) == 0)
		{
			int setValue = atoi(args[2]);
			PersistentConfigUtil::setStopIsShutdown((bool)setValue);
			return;
		}
#endif // DYNAMIC_STOP_BUTTON_BEHAVIOR

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
#endif // RELAY_SUPPORTED
	}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH || RELAY_SUPPORTED

} // namespace BasicCommands
