// SdCardSource.h

#ifndef _SdCardSource_h
#define _SdCardSource_h

#include "../../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM

#include <Arduino.h>
//#include <FS.h>

#include "StaticSecondaryDataSource.h"
#include "../../BottangoArduinoConfig.h"
#include "TxtBuffer.h"
#include "../Modules/AnimationPlaybackControl.h"

#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif // ESP32

enum class SdCmdType : uint8_t
{
	OpenFile,
	Stop,
};

class SdCardSource : public OfflineDataSource
{
public:
	struct SdCommand
	{
		SdCmdType type;
		char filePath[MAX_FILE_PATH_SIZE];
	};

	~SdCardSource();

	void onPhase(Phase p) override;
	void init() override;
	bool openSetup() override;
	bool openAnimation(uint8_t animIndex, bool loop) override;
	void prepareNextCommand() override;
	bool peekNextCommand(char* out) override;
	bool tryConsumeData(char** out) override;
	void resetBuffer() override;
	bool getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const override;

private:
	// ==== Buffer and file management ====
	char _commandBuffer[MAX_COMMAND_LENGTH];
	TxtBuffer<TXT_BUFFER_SIZE_SD> _cardReadBuffer;
	File _currentFile;

	uint8_t _index = 0;
	bool _onLoop = false;
	volatile bool _fileReadComplete = false;
	bool _shouldLoop = false;	

#ifdef ESP32
	volatile TaskHandle_t _fillTaskHandle = nullptr;
	void startFillTask();
	void stopFillTask();
	static void fillTask(void* param);
#else
	void updateOnLoop();
#endif

	bool fillBufferChunk();

	bool getNextCommand(char* buffer, bool peek = false);

	void parseConfiguration(File configFile, AnimationConfiguration* config) const;
};


#endif // USE_SD_CARD_COMMAND_STREAM
#endif // _SdCardSource_h