#include "ExportedAnimationPlaybackControl.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include "../System/BottangoCore.h"
#include "../DataSource/SDCardUtil.h"

namespace ExportedAnimationPlaybackControl
{
	CircularArray<AnimationConfiguration> animationConfigs = CircularArray<AnimationConfiguration>(MAX_EXPORTED_ANIMATIONS);
	int currentPlayingIndex = -1;
	int idleAnimIndex = -1;
	int startingAnim = -1;

	void initialize()
	{
#ifdef USE_SD_CARD_COMMAND_STREAM

		char path[MAX_FILE_PATH_SIZE];
		SDCardUtil::SDFileError fileError;

		// parse and build config files
		for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
		{
			// first verify existance of all required files
			path[0] = '\0';
			SDCardUtil::getAnimationFilePath(i, path, false, true); // Get and open CONFIG file
			bool exists = SDCardUtil::fileExists(path, fileError);
			if (!exists)
			{
				continue;
			}

			File configFile = SDCardUtil::openFile(path, fileError);

			// error case on config
			if (fileError != SDCardUtil::SDFileError::ERR_NONE)
			{
				// TODO
				// better error case handling here!
				SDCardUtil::closeFile(configFile);
				continue;
			}

			AnimationConfiguration* config = parseConfiguration(configFile);

			// done with the file
			SDCardUtil::closeFile(configFile);

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
#endif
			}

			animationConfigs.pushBack(config);
		}
#elif defined(USE_CODE_COMMAND_STREAM)
		for (int i = 0; i < GeneratedCodeAnimations::getAnimationCount(); i++)
		{
			const uint16_t* configValues = GeneratedCodeAnimations::getConfigValues(i);
			AnimationConfiguration* config = parseConfiguration(configValues);

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
#endif
			}
			animationConfigs.pushBack(config);
		}
#endif
		// look for starting animation to play and idle animation
		for (int i = 0; i < animationConfigs.size(); i++)
		{
			AnimationConfiguration* checkConfig = animationConfigs.get(i);
			if (checkConfig != nullptr)
			{
				if (checkConfig->playOnStart == 1 && startingAnim == -1)
				{
					startingAnim = i;
				}
				if (checkConfig->idleAnim == 1 && idleAnimIndex == -1)
				{
					idleAnimIndex = i;
				}
			}
		}
	}

	void updatePlaybackStatus()
	{
		// something already playing
		if (BottangoCore::commandStreamProvider->streamIsInProgress())
		{
			// let setup finish, don't interrupt it
			if (BottangoCore::commandStreamProvider->commandStreamIsSetup)
			{
				return;
			}
			// find an animation that wants to trigger
			int index = getIndexOfAnimationToTrigger();
			if (index >= 0 && index != currentPlayingIndex) // that isn't also the animation playing currently
			{
				AnimationConfiguration* nextConfig = animationConfigs.get(index);
				if (nextConfig != nullptr)
				{
					BottangoCore::commandStreamProvider->startCommandStream(index, nextConfig->loop > 0);
					currentPlayingIndex = index;
				}
			}
		}
		else
		{
			// nothing is playing
			if (currentPlayingIndex >= 0)
			{
				currentPlayingIndex = -1;
			}

			// something to trigger?
			int index = getIndexOfAnimationToTrigger();
			if (index >= 0)
			{
				AnimationConfiguration* nextConfig = animationConfigs.get(index);
				if (nextConfig != nullptr)
				{
					BottangoCore::commandStreamProvider->startCommandStream(index, nextConfig->loop > 0);
					currentPlayingIndex = index;
				}
			}
			// starting hasn't been triggered yet?
			else if (startingAnim >= 0 && currentPlayingIndex != startingAnim)
			{
				AnimationConfiguration* nextConfig = animationConfigs.get(startingAnim);
				if (nextConfig != nullptr)
				{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
					if (PersistentConfigUtil::debugEnabled())
#endif
					{
						Outgoing::printOutputStringFlash(F("trigger starting anim"));
						Outgoing::printLine();
					}
#endif
					BottangoCore::commandStreamProvider->startCommandStream(startingAnim, nextConfig->loopOnStart);
					currentPlayingIndex = idleAnimIndex;
					startingAnim = -1;
				}
			}
			// default to play when nothing should play?
			else if (idleAnimIndex >= 0 && currentPlayingIndex != idleAnimIndex)
			{
				AnimationConfiguration* nextConfig = animationConfigs.get(idleAnimIndex);
				if (nextConfig != nullptr)
				{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
					if (PersistentConfigUtil::debugEnabled())
#endif
					{
						Outgoing::printOutputStringFlash(F("trigger idle"));
						Outgoing::printLine();
					}
#endif
					BottangoCore::commandStreamProvider->startCommandStream(idleAnimIndex, true);
					currentPlayingIndex = idleAnimIndex;
				}
			}
		}
	}

#ifdef USE_SD_CARD_COMMAND_STREAM
	AnimationConfiguration* parseConfiguration(File configFile)
	{
		AnimationConfiguration* config = new AnimationConfiguration();
		byte lineIndex = 0;

		while (SDCardUtil::safeAvailable(configFile))
		{
			SDCardUtil::lockCard();
			// start of the line
			char c = configFile.read();
			SDCardUtil::unlockCard();

			// skip this line, it's a comment
			if (c == '/')
			{
				while (SDCardUtil::safeAvailable(configFile))
				{
					SDCardUtil::lockCard();
					c = configFile.read();
					SDCardUtil::unlockCard();
					if (c == '\n' || c == '\r')
					{
						break;
					}
				}
			}

			// skip this line, it's blank
			if (c == '\n' || c == '\r')
			{
				continue;
			}

			// valid line
			char value[10];
			value[0] = c;
			for (int i = 1; i < 10; i++)
			{
				if (SDCardUtil::safeAvailable(configFile))
				{
					SDCardUtil::lockCard();
					char cNext = configFile.read();
					SDCardUtil::unlockCard();
					if (cNext == '\n' || cNext == '\r')
					{
						value[i] = '\0';
						break;
					}
					else
					{
						value[i] = cNext;
						if (i == 8)
						{
							value[9] = '\0';
						}
					}
				}
			}

			int parsedValue = atoi(value);

			switch (lineIndex)
			{
			case 0:
				config->playOnStart = parsedValue;
				break;
			case 1:
				config->loopOnStart = parsedValue;
				break;
			case 2:
				config->idleAnim = parsedValue;
				break;
			case 3:
				config->playOnPin = parsedValue;
				break;
			case 4:
				config->loop = parsedValue;
				break;
			case 5:
				config->playOnPinHigh = parsedValue;
				break;
			case 6:
				config->buttonLadderMin = parsedValue;
				break;
			case 7:
				config->buttonLadderMax = parsedValue;

				break;
			}

			lineIndex++;
		}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			logConfig(config);
		}
#endif
		return config;
	}
#endif
#ifdef USE_CODE_COMMAND_STREAM
	AnimationConfiguration* parseConfiguration(const uint16_t* configValues)
	{
		AnimationConfiguration* config = new AnimationConfiguration();

		config->playOnStart = configValues[0];
		config->loopOnStart = configValues[1];
		config->idleAnim = configValues[2];
		config->playOnPin = configValues[3];
		config->loop = configValues[4];
		config->playOnPinHigh = configValues[5];
		config->buttonLadderMin = configValues[6];
		config->buttonLadderMax = configValues[7];

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			logConfig(config);
		}
#endif
		return config;
	}
#endif

	int getIndexOfAnimationToTrigger()
	{
		for (int i = 0; i < MAX_EXPORTED_ANIMATIONS; i++)
		{
			AnimationConfiguration* checkConfig = animationConfigs.get(i);
			if (checkConfig != nullptr && checkConfig->playOnPin > 0 && i != currentPlayingIndex)
			{
				if (checkConfig->playOnPinHigh == 0)
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
				else if (checkConfig->playOnPinHigh == 1)
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

#ifdef EXPORTED_ANIM_LOGGING
	void logConfig(AnimationConfiguration* config)
	{
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
}
#endif
