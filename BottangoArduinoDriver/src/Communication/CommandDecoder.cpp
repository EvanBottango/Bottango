// 
// 
// 

#include "CommandDecoder.h"


bool CommandDecoder::hasCommand()
{
	return validFrameAvailable;
}

bool CommandDecoder::tryConsumeCommand(char** out)
{
	if (validFrameAvailable)
	{
		out = splitCommandBuffer;
		return true;
	}

	return false;
}

void CommandDecoder::setDataSource(DataSource* src)
{
	source = src;
}