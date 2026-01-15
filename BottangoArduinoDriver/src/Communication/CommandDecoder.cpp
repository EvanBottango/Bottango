// 
// 
// 

#include "CommandDecoder.h"


bool CommandDecoder::hasCommand()
{
	return validCommandAvailable;
}

char** CommandDecoder::tryConsumeCommand()
{
	if (validCommandAvailable)
	{
		validCommandAvailable = false;
		return splitCommandBuffer;
	}

	return nullptr;
}

void CommandDecoder::setDataSource(DataSource* src)
{
	source = src;
}