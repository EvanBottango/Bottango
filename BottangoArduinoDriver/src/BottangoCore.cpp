#include "BottangoCore.h"

#include "../BottangoArduinoModules.h"
#include "Module Handling/ModuleMaster.h"
#include "DataSource/DataSource.h"
#include "System/SystemStatus.h"
#include "I2SAudioEffector.h"
#include "Modules/RelayComs/Relay.h"

namespace BottangoCore
{
	EffectorPool effectorPool = EffectorPool();
	AbstractMultiMessageOutgoingSource* activeOutgoingMultimessage = nullptr;

	ModuleMaster mMaster = ModuleMaster();

	char delimiters[] = ",";

#ifdef RELAY_LOGGING
	unsigned long lastWaitForConnectLog = 0;
#endif // RELAY_LOGGING

#if defined(RELAY_SUPPORTED)
	unsigned long lastPollTimeAsPeer;
#endif // RELAY_SUPPORTED

	void bottangoSetup()
	{
		//PersistentConfigUtil::setUseExportedCommandStream(true);
		//PersistentConfigUtil::setDebugEnabled(false);

		// Set the initial connection status. This can be overwritten by a module during initModules()
		SystemStatus::systemStatus.PowerStatus = SystemStatus::ePowerStatus::Ok;
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
		SystemStatus::systemStatus.Signal = SystemStatus::eSignal::SDError;

		mMaster.setupModules();
		mMaster.initModules();


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

		// init status lights
#ifdef ENABLE_STATUS_LIGHTS
		SystemStatus::systemStatus.Signal = SystemStatus::eSignal::Off;
		SystemStatus::systemStatus.UserLED = SystemStatus::eUserLED::Off;
#endif

		// Setup is done
		SystemStatus::systemStatus.initialized = true;
		Callbacks::onThisControllerStarted();
	}

	bool stopPlaybackModule(bool doUninitialize)
	{
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
		mMaster.getModule<AnimationPlaybackControl>(Modules::AnimPlaybackCntrl)->stop(doUninitialize);
#endif // USE_CODE_COMMAND_STREAM || USE_SD_CARD_COMMAND_STREAM

#if defined(RELAY_SUPPORTED)
		if (mMaster.getModule<Relay>(Modules::RelayComs)->stop(doUninitialize))
		{
			return false;
		}
#endif // RELAY_SUPPORTED
		return true;
	}

	static void hardStop()
	{
		Callbacks::onThisControllerStopped();
		uninitialize();

#ifdef ENABLE_STATUS_LIGHTS
#ifdef RELAY_SUPPORTED
		// peers reboot after getting stop
		if (mMaster.getModule<Relay>(Modules::RelayComs)->isPeer())
		{
			BasicCommands::reboot(false);
		}
		else
		{
			if (mMaster.getModule<Relay>(Modules::RelayComs)->isBridge())
			{
				SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Red;
			}
			else
			{
				SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
			}			
		}
#else 
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
#endif // RELAY_SUPPORTED
#endif // ENABLE_STATUS_LIGHTS
	}

	void request_Stop()
	{
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
		{
			stopPlaybackModule(false);
		}
		else
		{
			OutgoingSerial::printOutputStringPROGMEM(BasicCommands::STOP_PLAY);
		}
	}

	void request_eStop()
	{
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
		{
			if (stopPlaybackModule(true))
			{
				hardStop();
			}			
		}
		else
		{
			OutgoingSerial::printOutputStringPROGMEM(BasicCommands::ESTOP);
		}
	}

	void stop(bool doUninitialize)
	{
		// ToDo: doUninitialize is more or less unused with this version of stop(). Its only called from the Parser and is always true
		if (stopPlaybackModule(doUninitialize) && doUninitialize)
		{
			hardStop();
		}
	}

	void uninitialize()
	{
		BottangoCore::effectorPool.deregisterAll();
		while (Serial.available() > 0)
		{
			char t = Serial.read();
		}
		SystemStatus::systemStatus.initialized = false;

		DataSource* dataSource = mMaster.getModule<DataSource>(Modules::DataSource_Serial);
		dataSource->resetBuffer();
	}

	void bottangoLoop()
	{
		Callbacks::onEarlyLoop();
		mMaster.executePhase(Phase::Input);
		mMaster.executePhase(Phase::Communication);
		mMaster.executePhase(Phase::Logic);
		mMaster.executePhase(Phase::Output);


		if (SystemStatus::systemStatus.initialized)
		{
			// update relay pool
#ifdef RELAY_SUPPORTED
			if (mMaster.getModule<Relay>(Modules::RelayComs)->isPeer() && millis() - lastPollTimeAsPeer > RELAY_POLL_TIMEOUT_AS_PEER)
			{
				OutgoingSerial::printOutputStringFlash(F("Lost Bridge!"));
				OutgoingSerial::printLine();

				//BasicCommands::reboot(false);
				lastPollTimeAsPeer = millis();
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

#if defined(RELAY_SUPPORTED) && defined(RELAY_LOGGING)
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			if (mMaster.getModule<Relay>(Modules::RelayComs)->isPeer() && !SystemStatus::systemStatus.initialized && Time::getCurrentTimeInMs() - lastWaitForConnectLog >= 1000)
			{
				OutgoingSerial::printOutputStringFlash(F("Waiting for bridge...\n"));
				lastWaitForConnectLog = Time::getCurrentTimeInMs();
			}
		}
#endif // RELAY_SUPPORTED && RELAY_LOGGING
	}

} // namespace BottangoCore