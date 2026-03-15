// 
// 
// 

#include "Communication/CommandDecoder.h"


bool CommandDecoder::hasCommand()
{
	return validCommandAvailable;
}

char** CommandDecoder::tryConsumeCommand()
{
	// ToDo: Hier ist irgendwas seltsam? Wo ist splitCommandBuffer?
	if (validCommandAvailable)
	{
		validCommandAvailable = false;
		return splitCommandData.splitCommandBuffer;
	}

	return nullptr;
}

void CommandDecoder::setDataSource(DataSource* src)
{
	source = src;
}