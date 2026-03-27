// 
// 
// 

#include "Parser.h"
#include "../BasicCommands.h"
#include "../System/SystemStatus.h"

#ifdef RELAY_SUPPORTED
#include "../BottangoCore.h"
#include "../Modules/RelayComs/Relay.h"
#include "../Module Handling/ModuleMaster.h"
#endif

void Parser::onPhase(Phase p)
{
	// Only parse commands during the MainLoop phase
	if (p != Phase::Logic)
	{
		return;
	}

	/*if (parseCommand(splitCommandBuffer))
	{
		// Workaround for handshake bug - see SerialSource.cpp readData() for details
		Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
	}*/

	char** splitCommandBuffer = _decoder->tryConsumeCommand();
	bool sourceIsUsbSerial = _decoder->isSourceUsbSerial();

	if (splitCommandBuffer != nullptr)
	{
		while (splitCommandBuffer)
		{
			if (commandIsAllowed(splitCommandBuffer[0], sourceIsUsbSerial))
			{
				parseCommand(splitCommandBuffer); // Hier geht weiter: Müssen wir das sendReady hier beachten? Nochmal mit Evan abklären wegen der Reihenfolge, wann das OK\n wirklich kommen soll
				splitCommandBuffer = _decoder->tryConsumeCommand();
			}
		}

		// Workaround for handshake bug - see SerialSource.cpp readData() for details
		Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
	}

}

void Parser::setCommandDecoder(CommandDecoder* cmdDecoder)
{
	_decoder = cmdDecoder;
}

bool Parser::parseCommand(char** splitCommandBuffer) const
{
	if (splitCommandBuffer == nullptr)
	{
		return false;
	}

	// ToDo: LEDs and stuff
	//SystemStatus::systemStatus.CommandStatus = SystemStatus::eCommandStatus::NewCommand;

	// The command name is the first string in the array, subsequent strings are parameters of that command
	char* commandName = splitCommandBuffer[0];

	if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0)
	{
		BasicCommands::addCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::SET_INSTANTCURVE) == 0)
	{
		BasicCommands::addInstantCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::STOP) == 0)
	{
		BasicCommands::stop(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::TIME_SYNC) == 0)
	{
		BasicCommands::syncTime(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::DEREGISTER_ALL_EFFECTORS) == 0)
	{
		BasicCommands::deregisterAllEffectors(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
	{
		// ToDo: "secondary" temporary replaced with false
		BasicCommands::sendHandshakeResponse(splitCommandBuffer, false);
	}
	else if (strcmp_P(commandName, BasicCommands::MODULES_REQUEST) == 0)
	{
		BasicCommands::startModulesResponse(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::READY_FOR_NEXT_RESPONSE) == 0)
	{
		BasicCommands::continueInProgressMultiMessageResponse(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::CLEAR_ALL_CURVES) == 0)
	{
		BasicCommands::clearAllCurves(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::UPDATE_EFFECTOR_SIGNAL_BOUNDS) == 0)
	{
		BasicCommands::updateEffectorSignalBounds(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0)
	{
		BasicCommands::addOnOffCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0)
	{
		BasicCommands::addTriggerCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::CLEAR_EFFECTOR_CURVES) == 0)
	{
		BasicCommands::clearCurvesForEffector(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
	{
		BasicCommands::addColorCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::SET_INSTANT_COLOR_CURVE) == 0)
	{
		BasicCommands::addInstantColorCurve(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::DEREGISTER_EFFECTOR) == 0)
	{
		BasicCommands::deregisterEffector(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_I2C_SERVO) == 0)
	{
		BasicCommands::registerI2CServo(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_PIN_SERVO) == 0)
	{
		BasicCommands::registerPinServo(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_PIN_STEPPER) == 0)
	{
		BasicCommands::registerPinStepper(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_DIR_STEPPER) == 0)
	{
		BasicCommands::registerDirStepper(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_CURVED_EVENT) == 0)
	{
		BasicCommands::registerCurvedEvent(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_ONOFF_EVENT) == 0)
	{
		BasicCommands::registerOnOffEvent(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_TRIGGER_EVENT) == 0)
	{
		BasicCommands::registerTriggerEvent(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_CUSTOM_MOTOR) == 0)
	{
		BasicCommands::registerCustomMotor(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::REGISTER_COLOR_EVENT) == 0)
	{
		BasicCommands::registerColorEvent(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::STEPPER_SYNC) == 0)
	{
		BasicCommands::stepperSync(splitCommandBuffer);
	}
#ifdef AUDIO_SD_I2S
	else if (strcmp_P(commandName, BasicCommands::I2S_RegisterAudioEvent()) == 0)
	{
		BasicCommands::registerAudioEvent(splitCommandBuffer);
	}
	/*else if (strcmp_P(commandName, BasicCommands::AUDIO_BIN) == 0)
	{
		BasicCommands::processAudioBinary(splitCommandBuffer);
	}*/
#endif // AUDIO_SD_I2S
#ifdef RELAY_SUPPORTED
	else if (strcmp_P(commandName, BasicCommands::REGISTER_RELAY) == 0)
	{
		BasicCommands::registerRelayController(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::DEREGISTER_RELAY) == 0)
	{
		BasicCommands::deregisterRelayController(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::DEREGISTER_ALL_RELAY) == 0)
	{
		BasicCommands::deregisterAllRelayControllers(splitCommandBuffer);
	}
	else if (strcmp_P(commandName, BasicCommands::RELAY_HEARTBEAT_REQUEST) == 0)
	{
		BasicCommands::requestHeartbeat(splitCommandBuffer);
		//sendReady = false;
	}
	else if (strcmp_P(commandName, BasicCommands::REQUEST_PEER_BOOT) == 0)
	{
		BasicCommands::requestBoot(splitCommandBuffer);
	}
#ifdef RELAY_COMS_ESPNOW
	else if (strcmp_P(commandName, BasicCommands::GET_MAC_ADDRESS) == 0)
	{
		BasicCommands::getMACAddress(splitCommandBuffer);
	}
#endif // RELAY_COMS_ESPNOW
#endif // RELAY_SUPPORTED
#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED)
	else if (strcmp_P(commandName, BasicCommands::SET_CONFIG) == 0)
	{
		BasicCommands::setConfiguration(splitCommandBuffer);
	}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH || RELAY_SUPPORTED

	return true;
}

unsigned long Parser::getStartTime(char* command) const
{
	CommandDecoder::SplitCommandData data;
	data.stringToSplit = command;

	unsigned long startTime = (unsigned long)-1;
	bool commandWithTimeFound = false;

	// Split the incoming command
	// The splitCommand functions handles both regular and sync commands
	if (_decoder->splitCommand(&data))
	{
#ifdef ALLOW_SYNC_COMMANDS
		// Sync command handling
		if (data.syncCommandInProgress)
		{
			// With multi-frame sync commands, we need to check all frames for the earliest start time
			while (_decoder->hasMoreFrames(&data))
			{
				_decoder->getNextFrame(&data);

				// Split command and test if this sub-command has a start time
				// First parameter is command name, second start time, third is duration
				if (_decoder->splitCommand(&data) && commandHasStartTime(data.splitCommandBuffer[0]) && data.splitCommandBuffer[2] != nullptr)
				{
					unsigned long subCmdTime = strtoul(data.splitCommandBuffer[2], nullptr, 10);
					commandWithTimeFound = true;

					// Find the earliest start time
					if (subCmdTime < startTime)
					{
						startTime = subCmdTime;
					}
				}
			}

			*data.splitCommandBuffer[0] = '\0';	// Clear command to avoid re-processing
		}
#endif // ALLOW_SYNC_COMMANDS

		// To avoid re-processing when ALLOW_SYNC_COMMANDS is defined, splitCommandBuffer[0] is cleared above.
		// commandHasStartTime will return false if so.
		if (commandHasStartTime(data.splitCommandBuffer[0]) && data.splitCommandBuffer[2] != nullptr)
		{
			commandWithTimeFound = true;
			startTime = strtoul(data.splitCommandBuffer[2], nullptr, 10);
		}
	}

	// Command with start time found, return the calculated absolute time
	if (commandWithTimeFound)
	{
		return Time::getLastSyncedTimeInMs() + startTime;
	}
	// Even if no command with time was found, return at least the current time
	return Time::getCurrentTimeInMs();
}

unsigned long Parser::getEndTime(char* command) const
{
	CommandDecoder::SplitCommandData data;
	data.stringToSplit = command;

	bool commandWithTimeFound = false;
	unsigned long endTime = 0;

	// Split the incoming command
	// The splitCommand functions handles both regular and sync commands
	if (_decoder->splitCommand(&data))
	{
#ifdef ALLOW_SYNC_COMMANDS
		// Sync command handling
		if (data.syncCommandInProgress)
		{
			// With multi-frame sync commands, we need to check all frames for the earliest start time
			while (_decoder->hasMoreFrames(&data))
			{
				_decoder->getNextFrame(&data);

				// Split command and test if this sub-command has a duration
				// First parameter is command name, second start time, third is duration
				if (_decoder->splitCommand(&data)
					&& commandHasDuration(data.splitCommandBuffer[0])
					&& data.splitCommandBuffer[2] != nullptr
					&& data.splitCommandBuffer[3] != nullptr)
				{
					// We can assume, that every command with a duration, also has a start time
					unsigned long subCmdStartTime = strtoul(data.splitCommandBuffer[2], nullptr, 10);
					unsigned long subCmdDuration = strtoul(data.splitCommandBuffer[3], nullptr, 10);
					commandWithTimeFound = true;

					// Find the latest end time
					if (subCmdStartTime + subCmdDuration > endTime)
					{
						endTime = subCmdStartTime + subCmdDuration;
					}
				}
			}

			*data.splitCommandBuffer[0] = '\0';	// Clear command to avoid re-processing
		}
#endif // ALLOW_SYNC_COMMANDS


		// To avoid re-processing when ALLOW_SYNC_COMMANDS is defined, splitCommandBuffer[0] is cleared above.
		// commandHasStartTime will return false if so.
		if (commandHasDuration(data.splitCommandBuffer[0])
			&& commandHasDuration(data.splitCommandBuffer[0])
			&& data.splitCommandBuffer[2] != nullptr
			&& data.splitCommandBuffer[3] != nullptr)
		{
			commandWithTimeFound = true;
			// We can assume, that every command with a duration, also has a start time
			unsigned long startTime = Time::getLastSyncedTimeInMs() + strtoul(data.splitCommandBuffer[2], nullptr, 10);
			endTime = startTime + strtoul(data.splitCommandBuffer[3], nullptr, 10);
		}
	}

	// Command with end time found, return the calculated absolute time
	if (commandWithTimeFound)
	{
		return Time::getLastSyncedTimeInMs() + endTime;
	}
	// Even if no command with time was found, return at least the current time
	return Time::getCurrentTimeInMs();
}

bool Parser::commandHasStartTime(char* commandName) const
{
	if (commandName == nullptr || *commandName == '\0')
	{
		return false;
	}

	if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
		strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0 ||
		strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0 ||
		strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
	{
		return true;
	}

	return false;
}

bool Parser::commandHasDuration(char* commandName) const
{
	if (commandName == nullptr || *commandName == '\0')
	{
		return false;
	}

	if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0 ||
		strcmp_P(commandName, BasicCommands::SET_COLOR_CURVE) == 0)
	{
		return true;
	}

	return false;
}

bool Parser::commandIsAllowed(char* commandName, bool sourceIsUsbSerial) const
{
	bool limitiedCmdSet = false;

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
	if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
	{
		limitiedCmdSet = true;
	}
#endif

#ifdef RELAY_SUPPORTED
	if (sourceIsUsbSerial && BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getRole() == Relay::RelayRole::Peer)
	{
		limitiedCmdSet = true;
	} 
#endif // RELAY_SUPPORTED

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM) || defined(RELAY_SUPPORTED)
	if (limitiedCmdSet)
	{
		// Handshake request is allowed
		if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
		{
			return true;
		}
		// Modules request is allowed
		else if (strcmp_P(commandName, BasicCommands::MODULES_REQUEST) == 0)
		{
			return true;
		}
		// Continue modules request is allowed
		else if (strcmp_P(commandName, BasicCommands::READY_FOR_NEXT_RESPONSE) == 0)
		{
			return true;
		}
#ifdef ENABLE_ESP_OTA_UPDATE
		// ESP32 ota update is allowed
		else if (strcmp_P(commandName, BasicCommands::OTA_UPDATE) == 0)
		{
			return true;
		}
#endif // ENABLE_ESP_OTA_UPDATE
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
		// Switch configuration is allowed
		else if (strcmp_P(commandName, BasicCommands::SET_CONFIG) == 0)
		{
			return true;
		}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#ifdef RELAY_COMS_ESPNOW
		// Get MAC address is allowed
		else if (strcmp_P(commandName, BasicCommands::GET_MAC_ADDRESS) == 0)
		{
			return true;
		}
#endif

		// Otherwise ignore
		return false;
	}
#endif

	// allowed, no Export_Playback
	return true;
}