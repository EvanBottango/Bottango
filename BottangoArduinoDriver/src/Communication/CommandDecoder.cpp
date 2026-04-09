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

#if defined(RELAY_SUPPORTED)  || defined(USE_ESP32_WIFI)
void CommandDecoder::setSecondaryDataSource(DataSource* src)
{
	_secondarySource = src;
}
#endif;

void CommandDecoder::setOfflineDataSource(DataSource* src)
{
	_offlineSource = src;
}