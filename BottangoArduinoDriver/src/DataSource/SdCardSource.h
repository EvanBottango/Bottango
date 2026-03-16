// SdCardSource.h

#ifndef _SdCardSource_h
#define _SdCardSource_h

#include "../../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM

#include <Arduino.h>
//#include <FS.h>

#include "DataSource.h"
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

class SdCardSource : public DataSource
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

	bool openFile(const char* path);

	bool openSetup();

	bool openAnimation(uint8_t animIndex, bool loop);

	void prepareNextCommand();

	void peekNextCommand(char* out);

	bool tryConsumeData(char** out) override;

	void resetBuffer() override;

	bool getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const;

	void parseConfiguration(File configFile, AnimationConfiguration* config) const;

	bool dataComplete() const { return _dataComplete; };

private:
	QueueHandle_t _commandQueue;
	char _commandBuffer[MAX_COMMAND_LENGTH];
	TxtBuffer<TXT_BUFFER_SIZE_SD> _cardReadBuffer;
	File _currentFile;

	uint8_t _index = 0;
	bool _onLoop = false;
	volatile bool _fileReadComplete = false;
	bool _shouldLoop = false;
	bool _dataComplete = false;

#ifdef ESP32
	volatile TaskHandle_t _fillTaskHandle = nullptr;
	void startFillTask();
	static void fillTask(void* param);
#else
	void updateOnLoop();
#endif

	bool fillBufferChunk();

	void getNextCommand(char* buffer, bool peek = false);
};


#endif // USE_SD_CARD_COMMAND_STREAM
#endif // _SdCardSource_h