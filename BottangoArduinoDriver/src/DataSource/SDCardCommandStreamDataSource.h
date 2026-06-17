#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM

#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

#include "../../BottangoArduinoConfig.h"
#include "../Util/TxtBuffer.h"
#include "SDCardUtil.h"
#include "AbstractCommandStreamDataSource.h"

class SDCardCommandStreamDataSource : public AbstractCommandStreamDataSource
{
public:
	SDCardCommandStreamDataSource();
	SDCardCommandStreamDataSource(byte index, bool shouldLoop);

	virtual void getNextCommand(char* output, bool shouldLoop, bool peek) override;
	virtual void reset() override;
	virtual void updateOnLoop() override;
	~SDCardCommandStreamDataSource();

	bool isValid = false;

private:
	void checkIsValid();
	bool fillBufferChunk();

	byte index = 0;
	bool setup = false;
	bool onLoop = false;
	volatile bool cardReadComplete = false;
	bool shouldLoop = false;

#ifdef ESP32
	void startFillTask();
	static void fillTask(void* param);
	volatile TaskHandle_t fillTaskHandle = nullptr;
#endif

	TxtBuffer<TXT_BUFFER_SIZE_SD> cardReadBuffer;
	File currentFile;
};

#endif