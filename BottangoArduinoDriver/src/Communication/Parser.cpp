// 
// 
// 

#include "Parser.h"


bool Parser::parseCommand(FrameDecoder* decoder)
{
	// ToDo: Pointer to pointer is not needed anymore, this can be refactored to just use char*
	char** splitCommandBuffer = nullptr;

	if (decoder->tryConsumeFrame(splitCommandBuffer) == false)
	{
		return false;
	}

	//bool sendReady = true;

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
}