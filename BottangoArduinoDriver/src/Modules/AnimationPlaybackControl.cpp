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
#include "../Communication/CommandDecoder.h"

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
			// This way, calls that are more specific to a secondary data source don't need a #ifdef guard
#ifdef USE_SD_CARD_COMMAND_STREAM
			SdCardSource* secondarySource = BottangoCore::mMaster.getModule<SdCardSource>(Modules::DataSource_Secondary);
			CommandDecoder* decoder = BottangoCore::mMaster.getModule<CommandDecoder>(Modules::Decoder);

			// Peek the upcoming command to retrieve the endTime
			char nextCommand[MAX_COMMAND_LENGTH];
			nextCommand[0] = '\0';
			secondarySource->peekNextCommand(nextCommand);
			msEndOfLatestCommand = decoder->getEndTime(nextCommand);

			// Prepare the next command for the parser
			secondarySource->prepareNextCommand();
			
			// Peek the command after the upcoming command, to retrieve its startTime
			nextCommand[0] = '\0';
			secondarySource->peekNextCommand(nextCommand);
			timeOfNextCommand = decoder->getStartTime(nextCommand);
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
#elif USE_CODE_COMMAND_STREAM
	// ToDo for USE_CODE_COMMAND_STREAM
	// BottangoCore::mMaster.registerModuleInSecondaryDataSlot<ExportedCodeSource>();
#endif // USE_CODE_COMMAND_STREAM

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
	if (PersistentConfigUtil::getUseExportedCommandStream())
	{
		if (BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary) != nullptr)
		{
			// Activate the secondary data source
			BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary)->setActiveSource(true);
		}
	}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH


}

#ifdef USE_SD_CARD_COMMAND_STREAM
// ToDo: SD-Card specific code needs to be moved to SdCardSource
void AnimationPlaybackControl::loadConfig_SDCard()
{
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
}

AnimationPlaybackControl::AnimationConfiguration* AnimationPlaybackControl::parseConfiguration(File configFile)
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
#endif // USE_SD_CARD_COMMAND_STREAM

bool AnimationPlaybackControl::readyForNextCommand()
{
	// ToDo: Die Variablen aus der aktuellen config laden und hier verwursten
	// Ausnahme ist natürlich, wenn keine Animation läuft. Dann müssen hier default Werte geladen werden
	bool shouldLoop = false;
	bool dataComplete = false;

	// at end of loop
	if (shouldLoop /* && dataSource->dataComplete*/ && Time::getCurrentTimeInMs() >= msEndOfLatestCommand)
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
		timeOfNextCommand = 0;
		msEndOfLatestCommand = 0;

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
	if (timeOfNextCommand > SD_ANIM_PREREAD_MS)
	{
		return Time::getCurrentTimeInMs() >= timeOfNextCommand - SD_ANIM_PREREAD_MS;
	}
#endif // USE_SD_CARD_COMMAND_STREAM

	// otherwise all commands before pre-read are valid
	else if (timeOfNextCommand > 0)
	{
		return true;
	}
	// fallback if we're at or past time of next
	else
	{
		return Time::getCurrentTimeInMs() >= timeOfNextCommand;
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

#ifdef EXPORTED_ANIM_LOGGING
void AnimationPlaybackControl::logConfig(AnimationConfiguration* config)
{
	// ToDo: This is dobule guarded (see end of parseConfiguration)
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