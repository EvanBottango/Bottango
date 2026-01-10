// 
// 
// 

#include "AsciiParser.h"


bool AsciiParser::parseCommand(DataSource* source)
{
	char* commandString = nullptr;

	if (source->tryConsumeData(commandString) == false)
	{
		return false;
	}

	//bool sendReady = true;

	//SystemStatus::systemStatus.CommandStatus = SystemStatus::eCommandStatus::NewCommand;

#ifdef ALLOW_SYNC_COMMANDS
	// before split, check if this is a syncronized command
	// we don't actually want to split a syncronized command, but to parse it's own unique syntax
	if (strncmp_P(commandString, BasicCommands::SYNC_COMMAND, 3) == 0)
	{
		BasicCommands::executeSyncronizedCommands(commandString, secondary);
		return sendReady;
	}
#endif

	byte paramsCount = 0;
	bool splitSuccess = splitIntoBuffer(commandString, paramsCount);
	if (!splitSuccess)
	{
		return false;
		//return sendReady;
	}
}


bool AsciiParser::splitIntoBuffer(char* stringToSplit, byte& paramsCount)
{
	// Regular tokenization
	byte idxResult = 0;
	char* token = strtok(stringToSplit, delimiters);

	while (token != NULL)
	{
		if (idxResult >= COMMANDS_PARAMS_SIZE) // Check buffer capacity
		{
			Error::reportError_TooManyParams(idxResult);
			return false;
		}
		splitCommandBuffer[idxResult++] = token;
		token = strtok(NULL, delimiters);
	}

	paramsCount = idxResult;
	return true;
}