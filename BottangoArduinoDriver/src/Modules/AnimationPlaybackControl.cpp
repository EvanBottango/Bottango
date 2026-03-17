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

#include "../System/SystemStatus.h"

void AnimationPlaybackControl::onPhase(Phase p)
{
	if (p != Phase::Logic)
	{
		return;
	}

	// Check, if we are playing from PC or from a secondary data source
	if (_secondarySource && _secondarySource->isActiveSource())
	{
		// Ready for next command: prepare the next command
		if (readyForNextCommand())
		{
			// Note: Das sollte eigtl. auch mit EXPORTED_ANIM funktionieren
#ifdef USE_SD_CARD_COMMAND_STREAM
			if (complete())
			{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
				if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
				{
					Outgoing::printOutputStringFlash(F("stream complete"));
					Outgoing::printLine();
				}
#endif // EXPORTED_ANIM_LOGGING

				_setupIsRunning = false;
				stop();
				return;
			}

			// Peek the upcoming command to retrieve the endTime
			char nextCommand[MAX_COMMAND_LENGTH];
			nextCommand[0] = '\0';
			_secondarySource->peekNextCommand(nextCommand);

			// If the next commands runs longer than the current longest command, update the longest command end time
			unsigned long longestEndTime = _parser->getEndTime(nextCommand);
			if (longestEndTime > _timeEndOfLongestCommand)
			{
				_timeEndOfLongestCommand = longestEndTime;
			}

			// Prepare the next command for the parser
			_secondarySource->prepareNextCommand();
			
			// Peek the command after the upcoming command, to retrieve its startTime
			nextCommand[0] = '\0';
			_secondarySource->peekNextCommand(nextCommand);

			if (nextCommand[0] != '\0')
			{
				_timeStartOfNextCommand = _parser->getStartTime(nextCommand);
			}
#endif // USE_SD_CARD_COMMAND_STREAM
		}

		updatePlaybackStatus();
	}
}

void AnimationPlaybackControl::init()
{
	// Setup the secondary data source module, if any
#ifdef USE_SD_CARD_COMMAND_STREAM
	_secondarySource = static_cast<StaticSecondaryDataSource*>(BottangoCore::mMaster.registerModuleInSecondaryDataSlot<SdCardSource>());
	loadConfig();
	_secondarySource->openSetup();
	_setupIsRunning = true;
#elif USE_CODE_COMMAND_STREAM
	// ToDo for USE_CODE_COMMAND_STREAM
	// BottangoCore::mMaster.registerModuleInSecondaryDataSlot<ExportedCodeSource>();
#endif // USE_CODE_COMMAND_STREAM

	_parser = BottangoCore::mMaster.getModule<Parser>(Modules::Parser);

	// Look for starting animation to play and idle animation
	for (int i = 0; i < _animationConfigs.size(); i++)
	{
		AnimationConfiguration* checkConfig = _animationConfigs.get(i);
		if (checkConfig != nullptr)
		{
			if (ANIM_PLAY_ON_START(checkConfig->flags) == 1 && _startingAnim == -1)
			{
				_startingAnim = i;
			}
			if (ANIM_IS_IDLE_ANIM(checkConfig->flags) == 1 && _idleAnimIndex == -1)
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
		SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::NotPlaying;
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

	if (_secondarySource)
	{
		_secondarySource->resetBuffer();
	}

#ifdef RELAY_SUPPORTED
	if (BottangoCore::isRelayBridge)
	{
		BottangoCore::relayPool->clearCurvesOnConnectedPeers();
	}
#endif

	SystemStatus::systemStatus.Signal = SystemStatus::eSignal::OfflineReady;
}

void AnimationPlaybackControl::updatePlaybackStatus()
{
	// Don't interrupt the setup process
	if (_setupIsRunning)
	{
		return;
	}

	int index = getIndexOfAnimationToTrigger();

	// Something is playing
	if (SystemStatus::systemStatus.PlaybackStatus != SystemStatus::ePlaybackStatus::NotPlaying)
	{
		// That isn't also the animation playing currently
		if (index >= 0 && index != _currentPlayingIndex)
		{
			AnimationConfiguration* nextConfig = _animationConfigs.get(index);
			if (nextConfig != nullptr)
			{
				playAnimation(index, ANIM_IS_LOOPING(nextConfig->flags));
				_currentPlayingIndex = index;
			}
		}
	}
	// Nothing is playing
	else
	{	
		if (_currentPlayingIndex >= 0)
		{
			_currentPlayingIndex = -1;
		}

		// An animation wants to play from user input
		if (index >= 0)
		{
			AnimationConfiguration* nextConfig = _animationConfigs.get(index);
			if (nextConfig != nullptr)
			{
				playAnimation(index, ANIM_IS_LOOPING(nextConfig->flags));
				_currentPlayingIndex = index;
			}
		}
		// Starting animation hasn't been played yet
		else if (_startingAnim >= 0)
		{
			AnimationConfiguration* nextConfig = _animationConfigs.get(_startingAnim);
			if (nextConfig != nullptr)
			{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
				if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
				{
					Outgoing::printOutputStringFlash(F("trigger starting anim"));
					Outgoing::printLine();
				}
#endif // EXPORTED_ANIM_LOGGING
				playAnimation(_startingAnim, ANIM_LOOP_ON_START(nextConfig->flags));
				_currentPlayingIndex = _startingAnim;
				_startingAnim = -1;
			}
		}
		// Play idle animation if it exists and nothing else is playing
		else if (_idleAnimIndex >= 0)
		{
			AnimationConfiguration* nextConfig = _animationConfigs.get(_idleAnimIndex);
			if (nextConfig != nullptr)
			{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
				if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
				{
					Outgoing::printOutputStringFlash(F("trigger idle"));
					Outgoing::printLine();
				}
#endif // EXPORTED_ANIM_LOGGING
				_secondarySource->openAnimation(_idleAnimIndex, true);
				_currentPlayingIndex = _idleAnimIndex;
			}
		}
	}
}

#ifdef USE_SD_CARD_COMMAND_STREAM
void AnimationPlaybackControl::loadConfig()
{
	// parse and build config files
	for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
	{
		AnimationConfiguration* config = new AnimationConfiguration();
		if (!_secondarySource->getConfigurationForAnimation(i, config))
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
	// If the setup is running, the next command can be executed immediately
	if (_setupIsRunning)
	{
		return true;
	}

	bool shouldLoop = false;
	bool dataComplete = false;

	if (_currentPlayingIndex >= 0)
	{
		AnimationConfiguration* currentConfig = _animationConfigs.get(_currentPlayingIndex);
		if (currentConfig != nullptr)
		{
			shouldLoop = ANIM_IS_LOOPING(currentConfig->flags);
			dataComplete = _secondarySource->dataComplete();
		}
	}	

	// at end of loop
	if (shouldLoop && dataComplete && Time::getCurrentTimeInMs() >= _timeEndOfLongestCommand)
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
		_timeEndOfLongestCommand = 0;

		Time::syncTime(0);
		return true;
	}

	// at end of animation and not looping
	if (dataComplete)
	{
		return false;
	}

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
	if (_currentPlayingIndex >= 0 && ANIM_IS_LOOPING(_animationConfigs.get(_currentPlayingIndex)->flags))
	{
		return false;
	}

	return _secondarySource->dataComplete() && Time::getCurrentTimeInMs() >= _timeEndOfLongestCommand;
}

int AnimationPlaybackControl::getIndexOfAnimationToTrigger() const
{
	for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
	{
		AnimationConfiguration* checkConfig = _animationConfigs.get(i);
		if (checkConfig != nullptr && checkConfig->playOnPin > 0 && i != _currentPlayingIndex)
		{
			// Play on pin LOW
			if (ANIM_PLAY_ON_PIN_LOW(checkConfig->flags))
			{
				if (digitalRead(checkConfig->playOnPin) == LOW)
				{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
					if (PersistentConfigUtil::debugEnabled())
#endif
					{
						Outgoing::printOutputStringFlash(F("trigger pin low "));
						Outgoing::printOutputStringMem(checkConfig->playOnPin);
						Outgoing::printLine();
					}
#endif
					return i;
				}
			}
			// Play on pin HIGH
			else if (ANIM_PLAY_ON_PIN_HIGH(checkConfig->playOnPin))
			{
				if (digitalRead(checkConfig->playOnPin) == HIGH)
				{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
					if (PersistentConfigUtil::debugEnabled())
#endif
					{
						Outgoing::printOutputStringFlash(F("trigger pin high "));
						Outgoing::printOutputStringMem(checkConfig->playOnPin);
						Outgoing::printLine();
					}
#endif
					return i;
				}
			}
			// Play on pin ANALOG
			else
			{
				uint16_t pinV = analogRead(checkConfig->playOnPin);
				if (pinV >= checkConfig->buttonLadderMin && pinV <= checkConfig->buttonLadderMax)
				{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
					if (PersistentConfigUtil::debugEnabled())
#endif
					{
						Outgoing::printOutputStringFlash(F("trigger pin range "));
						Outgoing::printOutputStringMem(checkConfig->playOnPin);
						Outgoing::printOutputStringFlash(F(" : "));
						Outgoing::printOutputStringMem(pinV);
						Outgoing::printLine();
					}
#endif
					return i;
				}
			}
		}
	}

	return -1;
}

void AnimationPlaybackControl::playAnimation(int index, bool loop)
{
	if (SystemStatus::systemStatus.ConnectionStatus != SystemStatus::eConnectionStatus::Export_Playback)
	{
		return;
	}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::printOutputStringFlash(F("Start anim: "));
		Outgoing::printOutputStringMem(index);
		Outgoing::printLine();
	}
#endif

	stop();	

	SystemStatus::systemStatus.Signal = SystemStatus::eSignal::OfflinePlayback;

	_secondarySource->openAnimation(index, loop);

	Time::syncTime(0);
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
		Outgoing::printOutputStringMem(ANIM_PLAY_ON_START(config->flags));
		Outgoing::printOutputStringFlash(F(" "));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("loop on start: "));
		Outgoing::printOutputStringMem(ANIM_LOOP_ON_START(config->flags));
		Outgoing::printOutputStringFlash(F(" "));
		Outgoing::printLine();

		Outgoing::printOutputStringFlash(F("idle: "));
		Outgoing::printOutputStringMem(ANIM_IS_IDLE_ANIM(config->flags));
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
			Outgoing::printOutputStringMem(ANIM_IS_LOOPING(config->flags));
			Outgoing::printLine();
		}

		if (config->playOnPin > 0)
		{
			Outgoing::printOutputStringFlash(F("trigger type: "));
			if (ANIM_PLAY_ON_PIN_LOW(config->flags))
			{
				Outgoing::printOutputStringFlash(F("LOW"));
			}
			else if (ANIM_PLAY_ON_PIN_HIGH(config->flags))
			{
				Outgoing::printOutputStringFlash(F("HIGH"));
			}
			else
			{
				Outgoing::printOutputStringFlash(F("ANLG"));
			}
			Outgoing::printLine();
		}

		if (ANIM_PLAY_ON_PIN_ANALOG(config->flags))
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