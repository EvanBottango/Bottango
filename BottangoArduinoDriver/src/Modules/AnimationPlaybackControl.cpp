// 
// 
// 
#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include "AnimationPlaybackControl.h"
#include "../BottangoCore.h"
#include "../PersistentConfigUtil.h"
#include "../DataSource/SdCardSource.h"
#include "../Module Handling/ModuleMaster.h"
#include "../Outgoing.h"
#include "../Communication/Parser.h"
#include "../System/SystemStatus.h"

void AnimationPlaybackControl::onPhase(Phase p)
{
	if (p != Phase::Logic)
	{
		return;
	}

	// Check, if we are playing from PC or from a secondary data source
	if (BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary)->isActiveSource())
	{
		if (readyForNextCommand())
		{
			// ToDo: This call is specific to the SdCardSource - when going forward, it might be a good idea
			// to have a class chain like this: DataSource -> SecondaryDataSource -> SdCardSource
			// This way, calls that are more specific to a secondary data source don't need a #ifdef guard (like the getEndTime() or getStartTime() calls)
#ifdef USE_SD_CARD_COMMAND_STREAM
			SdCardSource* secondarySource = BottangoCore::mMaster.getModule<SdCardSource>(Modules::DataSource_Secondary);

			if (complete())
			{

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
				if (PersistentConfigUtil::debugEnabled())
#endif
				{
					Outgoing::printOutputStringFlash(F("stream complete"));
					Outgoing::printLine();
				}
#endif

				_setupIsRunning = false;
				stop();
				return;
			}

			Parser* parser = BottangoCore::mMaster.getModule<Parser>(Modules::Decoder);

			// Peek the upcoming command to retrieve the endTime
			char nextCommand[MAX_COMMAND_LENGTH];
			nextCommand[0] = '\0';
			secondarySource->peekNextCommand(nextCommand);
			_timeEndOfCurrentCommand = parser->getEndTime(nextCommand);

			// Prepare the next command for the parser
			secondarySource->prepareNextCommand();
			
			// Peek the command after the upcoming command, to retrieve its startTime
			nextCommand[0] = '\0';
			secondarySource->peekNextCommand(nextCommand);
			_timeStartOfNextCommand = parser->getStartTime(nextCommand);
#endif // USE_SD_CARD_COMMAND_STREAM
		}
	}
}

void AnimationPlaybackControl::init()
{
	// Setup the secondary data source module, if any
#ifdef USE_SD_CARD_COMMAND_STREAM
	BottangoCore::mMaster.registerModuleInSecondaryDataSlot<SdCardSource>();
	loadConfig_SDCard();
	BottangoCore::mMaster.getModule<SdCardSource>(Modules::DataSource_Secondary)->openSetup();
	_setupIsRunning = true;
#elif USE_CODE_COMMAND_STREAM
	// ToDo for USE_CODE_COMMAND_STREAM
	// BottangoCore::mMaster.registerModuleInSecondaryDataSlot<ExportedCodeSource>();
#endif // USE_CODE_COMMAND_STREAM

	// Look for starting animation to play and idle animation
	for (int i = 0; i < _animationConfigs.size(); i++)
	{
		AnimationConfiguration* checkConfig = _animationConfigs.get(i);
		if (checkConfig != nullptr)
		{
			if (checkConfig->playOnStart == 1 && _startingAnim == -1)
			{
				_startingAnim = i;
			}
			if (checkConfig->idleAnim == 1 && _idleAnimIndex == -1)
			{
				_idleAnimIndex = i;
			}
		}
	}

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
	// If expored animation should not play, return
	if (!PersistentConfigUtil::getUseExportedCommandStream())
	{
		return;
	}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH

	// Activate the secondary data source, if it exists
	if (BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary) != nullptr)
	{
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Export_Playback;
		BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary)->setActiveSource(true);
	}
}

void AnimationPlaybackControl::stop()
{
	if (SystemStatus::systemStatus.ConnectionStatus != SystemStatus::eConnectionStatus::Export_Playback)
	{
		return;
	}

	BottangoCore::effectorPool.clearAllCurves();

#ifdef RELAY_SUPPORTED
	if (BottangoCore::isRelayBridge)
	{
		BottangoCore::relayPool->clearCurvesOnConnectedPeers();
	}
#endif

	SystemStatus::systemStatus.Signal = SystemStatus::eSignal::OfflineReady;
}

#ifdef USE_SD_CARD_COMMAND_STREAM
void AnimationPlaybackControl::loadConfig_SDCard()
{
	// parse and build config files
	for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
	{
		SdCardSource* secondarySource = BottangoCore::mMaster.getModule<SdCardSource>(Modules::DataSource_Secondary);

		AnimationConfiguration* config = new AnimationConfiguration();
		if (!secondarySource->getConfigurationForAnimation(i, config))
		{
			delete config;
			continue;
		}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			logConfig(config);
		}
#endif		

		// set up pins if needed
		if (config->playOnPin > 0)
		{
			// boards with analog pins
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_SAM)
			if (config->playOnPin == 2)
			{
				pinMode(A0 + config->playOnPin, EXPORTED_ANIMATION_INPUT);
			}
			else
			{
				pinMode(config->playOnPin, EXPORTED_ANIMATION_INPUT);
			}
#else
			pinMode(config->playOnPin, EXPORTED_ANIMATION_INPUT);
#endif // defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_SAM)
		}

		_animationConfigs.pushBack(config);
	}
}
#endif // USE_SD_CARD_COMMAND_STREAM

bool AnimationPlaybackControl::readyForNextCommand()
{
	// ToDo: Die Variablen aus der aktuellen config laden und hier verwursten
	// Ausnahme ist natürlich, wenn keine Animation läuft. Dann müssen hier default Werte geladen werden
	bool shouldLoop = false;
	bool dataComplete = false;

	// at end of loop
	if (shouldLoop /* && dataSource->dataComplete*/ && Time::getCurrentTimeInMs() >= _timeEndOfCurrentCommand)
	{
		// reset to beginning
		BottangoCore::effectorPool.clearAllCurves();

/*#ifdef RELAY_SUPPORTED
		if (BottangoCore::isRelayBridge)
		{
			// Reset curves and time at end of loop before reset
			BottangoCore::relayPool->clearCurvesOnConnectedPeers();
			BottangoCore::relayPool->resumeTimeConnectedPeers(true);
		}
#endif*/
		//dataSource->reset(); // ToDo: dataSource zurücksetzen
		_timeStartOfNextCommand = 0;
		_timeEndOfCurrentCommand = 0;

		Time::syncTime(0);
		return true;
	}

	// at end of animation and not looping
	// ToDo: dataSource->dataComplete implementieren
	/*if (dataSource->dataComplete)
	{
		return false;
	}*/

#ifdef USE_SD_CARD_COMMAND_STREAM
/*#ifdef RELAY_SUPPORTED
	// not ready for next if esp comms pool is full
	if (BottangoCore::isRelayBridge && BottangoCore::relayPool->toPeerQueueFull())
	{
		return false;
	}

	// check pre read time?
	if (timeOfNextCommand > SD_ANIM_PREREAD_MS_RELAY)
	{
		return Time::getCurrentTimeInMs() >= timeOfNextCommand - SD_ANIM_PREREAD_MS;
	}
#else*/
	if (_timeStartOfNextCommand > SD_ANIM_PREREAD_MS)
	{
		return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand - SD_ANIM_PREREAD_MS;
	}
#endif // USE_SD_CARD_COMMAND_STREAM

	// otherwise all commands before pre-read are valid
	else if (_timeStartOfNextCommand > 0)
	{
		return true;
	}
	// fallback if we're at or past time of next
	else
	{
		return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand;
	}
/*#elif defined(USE_CODE_COMMAND_STREAM)
#ifdef RELAY_SUPPORTED
	// not ready for next if esp comms pool is full
	if (BottangoCore::isRelayBridge && BottangoCore::relayPool->toPeerQueueFull())
	{
		return false;
	}
#endif
	return Time::getCurrentTimeInMs() >= timeOfNextCommand;
#endif*/

	return false;
}

bool AnimationPlaybackControl::complete()
{
	// looping is never complete, needs to be canceled externally
	if (_animationConfigs.get(_currentPlayingIndex)->loop)
	{
		return false;
	}

	// ToDo: this needs to be moved to a new "secondaryDataSource" class. This only works for the SdCardSource, if we want to support more secondary data sources in the future,
	// we need a more generic way to check for completion that is not tied to a specific data source implementation.
	SdCardSource* secondarySource = BottangoCore::mMaster.getModule<SdCardSource>(Modules::DataSource_Secondary);
	return secondarySource->dataComplete() && Time::getCurrentTimeInMs() >= _timeEndOfCurrentCommand;
}

#ifdef EXPORTED_ANIM_LOGGING
void AnimationPlaybackControl::logConfig(AnimationConfiguration* config)
{
	// ToDo: This is double guarded (see end of parseConfiguration)
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::printOutputStringFlash(F("\nnew anim config"));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("play on start: "));
		Outgoing::printOutputStringMem(config->playOnStart > 0);
		Outgoing::printOutputStringFlash(F(" "));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("loop on start: "));
		Outgoing::printOutputStringMem(config->loopOnStart > 0);
		Outgoing::printOutputStringFlash(F(" "));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("idle: "));
		Outgoing::printOutputStringMem(config->idleAnim > 0);
		Outgoing::printOutputStringFlash(F(" "));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("play on pin: "));
		Outgoing::printOutputStringMem(config->playOnPin > 0);
		if (config->playOnPin > 0)
		{
			Outgoing::printOutputStringMem(config->playOnPin);
		}
		Outgoing::printLine();

		if (config->playOnPin > 0)
		{
			Outgoing::printOutputStringFlash(F("loop: "));
			Outgoing::printOutputStringMem(config->loop > 0);
			Outgoing::printLine();
		}

		if (config->playOnPin > 0)
		{
			Outgoing::printOutputStringFlash(F("trigger type: "));
			if (config->playOnPinHigh == 0)
			{
				Outgoing::printOutputStringFlash(F("LOW"));
			}
			else if (config->playOnPinHigh == 1)
			{
				Outgoing::printOutputStringFlash(F("HIGH"));
			}
			else
			{
				Outgoing::printOutputStringFlash(F("ANLG"));
			}
			Outgoing::printLine();
		}

		if (config->playOnPinHigh == 2)
		{
			Outgoing::printOutputStringFlash(F("range: "));
			Outgoing::printOutputStringMem(config->buttonLadderMin);
			Outgoing::printOutputStringFlash(F(" / "));
			Outgoing::printOutputStringMem(config->buttonLadderMax);
			Outgoing::printLine();
		}
	}
}
#endif

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)