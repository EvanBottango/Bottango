// SdCardSource.h

#ifndef _SdCardSource_h
#define _SdCardSource_h

#include "../../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM

#include <Arduino.h>
#include <FS.h>

#include "DataSource.h"
#include "../../BottangoArduinoConfig.h"
#include "TxtBuffer.h"

#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

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

	//void readData() override;

	void prepareNextCommand();

	void peekNextCommand(char* out);

	bool tryConsumeData(char** out) override;

	void resetBuffer() override;

private:
	QueueHandle_t commandQueue;
	char commandBuffer[MAX_COMMAND_LENGTH];
	TxtBuffer<TXT_BUFFER_SIZE_SD> cardReadBuffer;
	File currentFile;

	uint8_t index = 0;
	bool onLoop = false;
	volatile bool fileReadComplete = false;
	bool shouldLoop = false;
	bool dataComplete = false;

	//unsigned long timeOfNextCommand = 0;
	//unsigned long msEndOfLatestCommand = 0;

#ifdef ESP32
	volatile TaskHandle_t fillTaskHandle = nullptr;
	void startFillTask();
	static void fillTask(void* param);
#else
	void updateOnLoop();
#endif

	//void checkIsValid();

	bool fillBufferChunk();

	void getNextCommand(char* buffer, bool peek);

	//unsigned long getMSTimeOfCommand(char* commandString, bool returnStartTime);
};


#endif // USE_SD_CARD_COMMAND_STREAM
#endif // _SdCardSource_h