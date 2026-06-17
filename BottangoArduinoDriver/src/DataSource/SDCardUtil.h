#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)

#include <Arduino.h>
#include <SPI.h>
#include "SD.h"

#ifdef ESP32
#include <freertos/semphr.h>
#endif

namespace SDCardUtil
{
	enum SDFileError : uint8_t
	{
		ERR_NONE = 0, // success
		ERR_NO_CARD = 1,
		ERR_FILE_NOT_FOUND = 2,
		ERR_IO = 3,
	};

	File openFile(const char* filePath, SDFileError& fileError);
	File openFileForWrite(const char* filePath, SDFileError& fileError);
	bool fileExists(const char* filePath, SDFileError& fileError);
	ssize_t writeChunk(File& file, const uint8_t* data, size_t len, SDFileError& fileError);
	ssize_t writeChunk(File& file, const char* str, SDFileError& fileError);
	void closeFile(File& file);
	bool safeAvailable(File& file);

	void getAnimationFilePath(byte index, char* output, bool loop, bool config);
	void getAudioFilePath(char* identifier, char* output);
	void getAudioHashFilePath(char* identifier, char* output);
	void getSetupFilePath(char* output);

	extern unsigned long lastMountAttemptTime;
	extern bool internalSDMounted;
	extern bool cardLost;

#ifdef ESP32

	// wrap all sd card operations in a lock / unlock to ensure task safe
	extern SemaphoreHandle_t sdMutex;
	static inline void lockCard()
	{
		if (!sdMutex)
		{
			sdMutex = xSemaphoreCreateMutex();
		}
		xSemaphoreTake(sdMutex, portMAX_DELAY);
	}
	static inline void unlockCard() { xSemaphoreGive(sdMutex); }

	extern SPIClass spi;
#else
	static inline void lockCard()
	{
		// no op
	}
	static inline void unlockCard()
	{
		// no op
	}
#endif

} // namespace SDCardUtil
#endif