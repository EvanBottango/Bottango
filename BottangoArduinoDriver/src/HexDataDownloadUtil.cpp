#include "../BottangoArduinoModules.h"
#include "HexDataDownloadUtil.h"

#if defined(ENABLE_ESP_OTA_UPDATE)
#include "Outgoing.h"

namespace HexDataDownloadUtil
{
	DataReceivedCallback dataCallback = nullptr;
	bool downloadInProgress = false;
	uint32_t checksum = 0;

	void beginData(DataReceivedCallback callback)
	{
		checksum = 0;
		downloadInProgress = true;
		dataCallback = callback;
	}

	void recvData(const char* hexData)
	{
		if (!downloadInProgress)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: HexData-NotStarted"));
			Outgoing::printLine();
			return;
		}

		int len = strlen(hexData);
		if (len % 2 != 0)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: HexData-BadHex"));
			Outgoing::printLine();
			return;
		}

		int dataLen = len / 2;
		uint8_t* buffer = (uint8_t*)malloc(dataLen);
		if (!buffer)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: HexData-MemAlloc"));
			Outgoing::printLine();
			return;
		}

		// Convert hex string to binary data
		for (int i = 0; i < dataLen; i++)
		{
			char c1 = hexData[i * 2];
			char c2 = hexData[i * 2 + 1];
			char temp[3] = { c1, c2, '\0' };
			buffer[i] = (uint8_t)strtol(temp, NULL, 16);
		}

		// Write the chunk to flash
		if (dataCallback)
		{
			dataCallback(buffer, dataLen);
		}

		// Update checksum
		for (int i = 0; i < dataLen; i++)
		{
			checksum += buffer[i];
		}

		free(buffer);
	}

	void finishData()
	{
		checksum = 0;
		downloadInProgress = false;
		dataCallback = nullptr;
	}
}

#endif