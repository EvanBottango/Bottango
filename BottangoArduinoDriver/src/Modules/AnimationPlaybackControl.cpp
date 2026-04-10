#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include "AnimationPlaybackControl.h"
#include "../BottangoCore.h"
#include "../PersistentConfigUtil.h"
#include "../DataSource/SdCardSource.h"
#include "../DataSource/CodeSource.h"
#include "../Module Handling/ModuleMaster.h"
#include "Modules/Outgoing.h"
#include "../Communication/CommandDecoder.h"


#include "../System/SystemStatus.h"

#include "Logger/Logger.h"

void AnimationPlaybackControl::onPhase(Phase p)
{
	if (p != Phase::Logic)
	{
		return;
	}

	// Note: Das sollte eigtl. auch mit EXPORTED_ANIM funktionieren

	// Check, if we are playing from PC or from a secondary data source
	if (_offlineSource == nullptr || !_offlineSource->isActiveSource())
	{
		return;
	}

	// Ready for next command: prepare the next command
	if (readyForNextCommand())
	{
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
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

			LOG_DEBUG("APC", "onPhase()", "Command stream complete, stopping playback");
			_setupIsRunning = false;
			BottangoCore::request_Stop();
			return;
		}

#ifdef RELAY_SUPPORTED
		if (_relay->isBridge())
		{
			// need to wait for the to peer queue to clear out?
			if (_relay->getPeerPool()->toPeerQueueFull())
			{
				return;
			}

			// Do not continue during setup when not all peers are connected yet
			if (_setupIsRunning && _registeredAllPeers && !_relay->getPeerPool()->bridgeIsConnectedToAllPeers())
			{
				return;
			}
		}
#endif // RELAY_SUPPORTED

		// Peek the upcoming command to retrieve the endTime
		char nextCommand[MAX_COMMAND_LENGTH];
		nextCommand[0] = '\0';

		if (_offlineSource->peekNextCommand(nextCommand))
		{
#ifdef RELAY_SUPPORTED
			if (_setupIsRunning && !_registeredAllPeers)
			{
				// The first lines are always REGISTER_RELAY commands. If we encounter a different command, we can assume that all peers have been registered
				if (strncmp(nextCommand, BasicCommands::REGISTER_RELAY, sizeof(BasicCommands::REGISTER_RELAY) - 1) != 0)
				{
					_registeredAllPeers = true;
					return;
				}
			}
#endif // RELAY_SUPPORTED

			LOG_DEBUG("APC", "onPhase()", "Next Command: %s", nextCommand);
			// If the next commands runs longer than the current longest command, update the longest command end time
			unsigned long longestEndTime = _parser->getEndTime(nextCommand);
			if (longestEndTime > _timeEndOfLongestCommand)
			{
				_timeEndOfLongestCommand = longestEndTime;
			}

			// Prepare the next command for the parser
			// The parser will retrieve the command during its own logic phase
			_offlineSource->prepareNextCommand();

			// Peek the command after the upcoming command, to retrieve its startTime
			nextCommand[0] = '\0';
			if (_offlineSource->peekNextCommand(nextCommand))
			{
				_timeStartOfNextCommand = _parser->getStartTime(nextCommand);
			}
		}
#endif // USE_SD_CARD_COMMAND_STREAM
	}

	updatePlaybackStatus();

	/*#ifdef RELAY_SUPPORTED
		// If we're a relay bridge, we need to check if we should resume time on connected peers after setup
		if (!_peerSetupDone && !_setupIsRunning && _relay->isBridge() && _relay->getPeerPool()->bridgeIsConnectedToAllPeers())
		{
			_relay->getPeerPool()->resumeTimeConnectedPeers(false);
			Time::syncTime(0);
			_peerSetupDone = true;
		}
	#endif*/ // RELAY_SUPPORTED
}

void AnimationPlaybackControl::init()
{
#ifdef RELAY_SUPPORTED
	_relay = BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs);

	// We are no bridge, so we can skip all the setup
	if (!_relay->isBridge())
	{
		return;
	}
#endif // RELAY_SUPPORTED

	// Note: Unsure about the best place for this. Normaly it should live in the ModuleMaster.
	// But it also feels like it would fit here perfectly

	// Setup the secondary data source module, if any
#ifdef USE_SD_CARD_COMMAND_STREAM
	_offlineSource = static_cast<OfflineDataSource*>(BottangoCore::mMaster.registerModuleInOfflineDataSlot<SdCardSource>());
#elif defined(USE_CODE_COMMAND_STREAM)
	_offlineSource = static_cast<OfflineDataSource*>(BottangoCore::mMaster.registerModuleInOfflineDataSlot<CodeSource>());
#endif // USE_CODE_COMMAND_STREAM

	loadConfig();
	// ToDo: Error handling when openSetup fails
	_offlineSource->openSetup();
	_setupIsRunning = true;

	BottangoCore::mMaster.getModule<CommandDecoder>(Modules::Decoder)->setOfflineDataSource(_offlineSource);

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

	// Activate the offline data source, if it exists
	if (BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Offline) != nullptr)
	{
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::Export_Playback;
		SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::NotPlaying;
		BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Offline)->setActiveSource(true);
	}
}

void AnimationPlaybackControl::stop(bool allowSetupStop)
{
	// Don't stop if we're not playing from the offline source or if we're in the middle of setup and not allowed to stop it
	if (SystemStatus::systemStatus.ConnectionStatus != SystemStatus::eConnectionStatus::Export_Playback
		|| (_setupIsRunning && !allowSetupStop))
	{
		return;
	}

	if (allowSetupStop)
	{
		setInvalidState();
	}

	BottangoCore::effectorPool.clearAllCurves();
	_timeStartOfNextCommand = 0;
	_timeEndOfLongestCommand = 0;
	_currentPlayingIndex = -1;

	if (_offlineSource)
	{
		_offlineSource->resetBuffer();
	}

	// ToDo: A approach is needed when loosing one peer after initial setup
	//_offlineSource->openSetup();
	//_setupIsRunning = true;

/*#ifdef RELAY_SUPPORTED
	if (BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->isBridge())
	{
		//_peerSetupDone = false; // ToDo: A approach is needed when loosing one peer after initial setup
		BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->clearCurvesOnConnectedPeers();
	}
#endif*/

	SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::NotPlaying;
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
				SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::PlayingOtherAnimation;
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
				SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::PlayingOtherAnimation;
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
				SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::PlayingStartAnimation;
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
				_offlineSource->openAnimation(_idleAnimIndex, true);
				SystemStatus::systemStatus.PlaybackStatus = SystemStatus::ePlaybackStatus::PlayingIdleAnimation;
				_currentPlayingIndex = _idleAnimIndex;
			}
		}
	}
}

//#ifdef USE_SD_CARD_COMMAND_STREAM
void AnimationPlaybackControl::loadConfig()
{
	// parse and build config files
	for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
	{
		AnimationConfiguration* config = new AnimationConfiguration();
		if (!_offlineSource->getConfigurationForAnimation(i, config))
		{
			delete config;
			continue;
		}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			logConfig(config);
		}
#endif // EXPORTED_ANIM_LOGGING

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
//#endif // USE_SD_CARD_COMMAND_STREAM

bool AnimationPlaybackControl::readyForNextCommand()
{
	// If the setup is running, the next command can be executed immediately
	if (_setupIsRunning)
	{
		return true;
	}

	if (SystemStatus::systemStatus.PlaybackStatus == SystemStatus::ePlaybackStatus::NotPlaying)
	{
		return false;
	}

	bool shouldLoop = false;
	bool dataComplete = false;

	if (_currentPlayingIndex >= 0)
	{
		AnimationConfiguration* currentConfig = _animationConfigs.get(_currentPlayingIndex);
		if (currentConfig != nullptr)
		{
			shouldLoop = ANIM_IS_LOOPING(currentConfig->flags);
			dataComplete = _offlineSource->dataComplete();
		}
	}

	// at end of loop
	if (shouldLoop && dataComplete && Time::getCurrentTimeInMs() >= _timeEndOfLongestCommand)
	{
		// reset to beginning
		BottangoCore::effectorPool.clearAllCurves();

#ifdef RELAY_SUPPORTED
		if (_relay->isBridge())
		{
			// Reset curves and time at end of loop before reset
			_relay->getPeerPool()->clearCurvesOnConnectedPeers();
			_relay->getPeerPool()->resumeTimeConnectedPeers(true);
		}
#endif // RELAY_SUPPORTED

		/*if (_offlineSource)
		{
			_offlineSource->resetBuffer();
		}*/

		_timeStartOfNextCommand = 0;
		_timeEndOfLongestCommand = 0;

		Time::syncTime(0);
		return true;
	}

	// at end of animation and not looping
	/*if (dataComplete)
	{
		return false;
	}*/


#ifdef RELAY_SUPPORTED
	// not ready for next if esp comms pool is full
	if (_relay->isBridge() && _relay->getPeerPool()->toPeerQueueFull())
	{
		return false;
	}
#endif

#ifdef USE_SD_CARD_COMMAND_STREAM
#ifdef RELAY_SUPPORTED
	// check pre read time?
	if (_timeStartOfNextCommand > SD_ANIM_PREREAD_MS_RELAY)
	{
		return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand - SD_ANIM_PREREAD_MS;
	}
#else
	if (_timeStartOfNextCommand > SD_ANIM_PREREAD_MS)
	{
		return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand - SD_ANIM_PREREAD_MS;
	}
#endif // RELAY_SUPPORTED


	// otherwise all commands before pre-read are valid
	if (_timeStartOfNextCommand > 0)
	{
		return true;
	}
	// fallback if we're at or past time of next
	return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand;

//#endif // USE_SD_CARD_COMMAND_STREAM
#elif defined(USE_CODE_COMMAND_STREAM)
#ifdef RELAY_SUPPORTED
	// not ready for next if esp comms pool is full
	if (BottangoCore::isRelayBridge && BottangoCore::relayPool->toPeerQueueFull())
	{
		return false;
	}
#endif // RELAY_SUPPORTED
	return Time::getCurrentTimeInMs() >= _timeStartOfNextCommand;
#endif // USE_CODE_COMMAND_STREAM

	//return false;
}

bool AnimationPlaybackControl::complete()
{
	// looping is never complete, needs to be canceled externally
	if (_currentPlayingIndex >= 0 && ANIM_IS_LOOPING(_animationConfigs.get(_currentPlayingIndex)->flags))
	{
		return false;
	}

	auto debugTime = 0;
	bool complete = _offlineSource->dataComplete();
	if (_offlineSource->dataComplete() && Time::getCurrentTimeInMs() >= _timeEndOfLongestCommand)
	{
		return true;
	}

	return _offlineSource->dataComplete() && Time::getCurrentTimeInMs() >= _timeEndOfLongestCommand;
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
			else if (ANIM_PLAY_ON_PIN_HIGH(checkConfig->flags))
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
	if (SystemStatus::systemStatus.ConnectionStatus != SystemStatus::eConnectionStatus::Export_Playback || _invalidState)
	{
		return;
	}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
	{
		Outgoing::printOutputStringFlash(F("Start anim: "));
		Outgoing::printOutputStringMem(index);
		Outgoing::printLine();
	}
#endif // EXPORTED_ANIM_LOGGING

	BottangoCore::request_Stop();
	LOG_DEBUG("APC", "playAnimation()", "Playing animation index %d, loop: %d", index, loop);
	SystemStatus::systemStatus.Signal = SystemStatus::eSignal::OfflinePlayback;

	// Opening the animation was successful
	if (_offlineSource->openAnimation(index, loop))
	{
#ifdef RELAY_SUPPORTED
		if (BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->isBridge())
		{
			// ToDo: Do we always send the whole setup again for each animation we play, or should this be a one-time only?
			// There should be a check, if we lost a peer in the meantime, to only resend setup data if needed

			//Time::stopTime();
			//BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->stopTimeOnConnectedPeers();

			_relay->getPeerPool()->resumeTimeConnectedPeers(false);
			//Time::syncTime(0);
			//return;
		}
#endif // RELAY_SUPPORTED

		Time::syncTime(0);
	}
}

void AnimationPlaybackControl::setInvalidState()
{
	_invalidState = true;
	SystemStatus::systemStatus.Signal = SystemStatus::eSignal::SDError;
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