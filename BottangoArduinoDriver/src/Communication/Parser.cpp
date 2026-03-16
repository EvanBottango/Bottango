// 
// 
// 

#include "Parser.h"
#include "../BasicCommands.h"

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

	if (splitCommandBuffer != nullptr)
	{
		while (splitCommandBuffer)
		{
			parseCommand(splitCommandBuffer);
			splitCommandBuffer = _decoder->tryConsumeCommand();
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
		// ToDo: secondary temporary replaced with false
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
	return startTime;
}

unsigned long Parser::getEndTime(char* command) const
{
	CommandDecoder::SplitCommandData data;
	data.stringToSplit = command;

	// Split the incoming command
	// The splitCommand functions handles both regular and sync commands
	if (_decoder->splitCommand(&data))
	{
		unsigned long endTime = 0;

		// First parameter is command name, third is end time
		if (data.splitCommandBuffer[3] != nullptr)
		{
			endTime = strtoul(data.splitCommandBuffer[3], nullptr, 10);

#ifdef ALLOW_SYNC_COMMANDS
			// With multi-frame sync commands, we need to check all frames for the latest end time
			while (_decoder->hasMoreFrames(&data))
			{
				_decoder->getNextFrame(&data);
				unsigned long subCmdTime = strtoul(data.splitCommandBuffer[3], nullptr, 10);
				if (subCmdTime > endTime)
				{
					endTime = subCmdTime;
				}
			}
#endif // ALLOW_SYNC_COMMANDS

			return Time::getLastSyncedTimeInMs() + endTime;
		}
	}

	return 0;
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

bool Parser::commandHasEndTime(char* commandName) const
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