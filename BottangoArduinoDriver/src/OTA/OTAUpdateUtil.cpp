#include "OTAUpdateUtil.h"

#if defined(ENABLE_ESP_OTA_UPDATE)
#include "../Communication/Outgoing.h"
#include "../Communication/BasicCommands.h"
#include "HexDataDownloadUtil.h"

namespace OTAUpdateUtil
{
	bool otaInProgress = false;

	void beginOTA()
	{
		if (otaInProgress)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: OTA-InProgress"));
			Outgoing::printLine();
			return;
		}

		if (!Update.begin(UPDATE_SIZE_UNKNOWN))
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: OTA-InitFail"));
			Outgoing::printLine();
			return;
		}

		if (HexDataDownloadUtil::downloadInProgress)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: Already Download"));
			Outgoing::printLine();
			return;
		}

		Outgoing::printOutputStringFlash(F("OTA Begin!"));
		Outgoing::printLine();
		HexDataDownloadUtil::beginData(processHexData);
		otaInProgress = true;
	}

	void recvOTAData(const char* hexData)
	{
		if (!otaInProgress)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: OTA-NotStarted"));
			Outgoing::printLine();
			return;
		}

		HexDataDownloadUtil::recvData(hexData);
	}

	void processHexData(uint8_t* buffer, size_t dataLength)
	{
		// Write the chunk to flash
		size_t written = Update.write(buffer, dataLength);
		if (written != (size_t)dataLength)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: OTA-WriteFail"));
			Outgoing::printLine();
		}
	}

	void finishOTA(const char* expectedChecksumStr)
	{
		if (!otaInProgress || !HexDataDownloadUtil::downloadInProgress)
		{
			// error here
			Outgoing::printOutputStringFlash(F("ERR: OTA-NotStarted"));
			Outgoing::printLine();
			return;
		}

		uint32_t expectedChecksum = (uint32_t)strtoul(expectedChecksumStr, NULL, 10);

		if (HexDataDownloadUtil::checksum == expectedChecksum)
		{
			HexDataDownloadUtil::finishData();

			// Finalize the update
			if (Update.end(true))
			{
				otaInProgress = false;
				Outgoing::printOutputStringFlash(F("OTA Success, Restarting"));
				Outgoing::printLine();
				Outgoing::printOutputStringPROGMEM(BasicCommands::READY);

				// yes, delay on purpose. Blocking the thread is desired here, I don't want anything else to come in
				// this should be the only place delay is actually correct in this codebase...
				delay(1000);
				ESP.restart();
			}
			else
			{
				Outgoing::printOutputStringFlash(F("ERR: OTA-EndFail: "));
				Outgoing::printOutputStringMem(Update.getError());
				Outgoing::printLine();
				otaInProgress = false;
				Update.abort();
			}
		}
		else
		{
			Outgoing::printOutputStringFlash(F("ERR: OTA-BadChecksum"));
			Outgoing::printLine();
			otaInProgress = false;
			Update.abort();
			HexDataDownloadUtil::finishData();
		}
	}
}
#endif