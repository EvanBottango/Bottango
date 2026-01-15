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

	parseCommand();
}

void Parser::setCommandDecoder(CommandDecoder* cmdDecoder)
{
	decoder = cmdDecoder;
}

bool Parser::parseCommand()
{
	// ToDo: Pointer to pointer is not needed anymore, this can be refactored to just use char*
	char** splitCommandBuffer = decoder->tryConsumeCommand();
	//char** splitCommandBuffer;// = decoder->splitCommandBuffer;

	//if (decoder->tryConsumeCommand(splitCommandBuffer) == false)
	if (splitCommandBuffer == nullptr)
	{
		return false;
	}

	//Outgoing::printOutputStringMem("Parsing\n");
	/*Serial.printf("Parser Start\n");
	Serial.printf("Command 0: %s\n", splitCommandBuffer[0]);
	Serial.printf("Command 1: %s\n", splitCommandBuffer[1]);
	Serial.printf("Command 2: %s\n", splitCommandBuffer[2]);
	Serial.flush();*/

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

	//Serial.printf("Parser End\n");
	//Serial.flush();

	return true;
}