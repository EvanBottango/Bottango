#include "../BottangoArduinoModules.h"
#include "AudioBinaryUtil.h"

#if defined(AUDIO_SD_I2S)
#include "Outgoing.h"
#include "BasicCommands.h"
#include "HexDataDownloadUtil.h"

namespace AudioBinaryUtil
{
	bool audioBinaryInProgress = false;
	File writeFile;

	void beginAudioBinary(char* audioIdentifier, bool isHash)
	{
		if (audioBinaryInProgress)
		{
			// error here
			return;
		}

		if (HexDataDownloadUtil::downloadInProgress)
		{
			// error here
			return;
		}

		char filePathBuffer[MAX_FILE_PATH_SIZE];
		filePathBuffer[0] = '\0';
		if (isHash)
		{
			SDCardUtil::getAudioHashFilePath(audioIdentifier, filePathBuffer);
		}
		else
		{
			SDCardUtil::getAudioFilePath(audioIdentifier, filePathBuffer);
		}

		SDCardUtil::SDFileError fileError;
		writeFile = SDCardUtil::openFileForWrite(filePathBuffer, fileError);

		if (fileError != SDCardUtil::SDFileError::ERR_NONE)
		{
			// error here
			return;
		}

		HexDataDownloadUtil::beginData(processAudioBinaryData);
		audioBinaryInProgress = true;
	}

	void recvAudioBinaryData(const char* hexData)
	{
		if (!audioBinaryInProgress)
		{
			// error here
			return;
		}
		HexDataDownloadUtil::recvData(hexData);
	}

	void processAudioBinaryData(uint8_t* buffer, size_t dataLength)
	{
		// Write the chunk to sd
		SDCardUtil::SDFileError fileErr;
		SDCardUtil::writeChunk(writeFile, buffer, dataLength, fileErr);
	}

	void finishAudioBinary(const char* expectedChecksumStr)
	{
		if (!audioBinaryInProgress || !HexDataDownloadUtil::downloadInProgress)
		{
			// error here
			return;
		}

		uint32_t expectedChecksum = (uint32_t)strtoul(expectedChecksumStr, NULL, 10);

		if (HexDataDownloadUtil::checksum == expectedChecksum)
		{
			// Finalize the write operation on success
			SDCardUtil::closeFile(writeFile);
		}
		else
		{
			// Finalize the write operation on bad final checksum
			SDCardUtil::closeFile(writeFile);
		}
		HexDataDownloadUtil::finishData();
	}
}
#endif