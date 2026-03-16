// 
// 
// 

#include "Communication/CommandDecoder.h"


bool CommandDecoder::hasCommand()
{
	return _validCommandAvailable;
}

char** CommandDecoder::tryConsumeCommand()
{
	// ToDo: Hier ist irgendwas seltsam? Wo ist splitCommandBuffer?
	if (_validCommandAvailable)
	{
		_validCommandAvailable = false;
		return _splitData.splitCommandBuffer;
	}

	return nullptr;
}

void CommandDecoder::setDataSource(DataSource* src)
{
	_source = src;
}