#include "CommandDecoder.h"


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

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
void CommandDecoder::setOfflineDataSource(DataSource* src)
{
	_offlineSource = src;
}
#endif // ALLOW_SYNC_COMMANDS