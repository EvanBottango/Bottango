#include "../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM
#ifndef SDCardCommandStreamDataSource_h
#define SDCardCommandStreamDataSource_h

#include "AbstractCommandStreamDataSource.h"
#include "../BottangoArduinoConfig.h"
#include "DataSource/SDCardUtil.h"
#include "DataSource/TxtBuffer.h"
//#include <FS.h>

#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

class SDCardCommandStreamDataSource : public AbstractCommandStreamDataSource
{
public:
    SDCardCommandStreamDataSource();
    SDCardCommandStreamDataSource(byte index, bool shouldLoop);

    virtual void getNextCommand(char *output, bool shouldLoop, bool peek) override;
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
    static void fillTask(void *param);
    volatile TaskHandle_t fillTaskHandle = nullptr;
#endif

    TxtBuffer<TXT_BUFFER_SIZE_SD> cardReadBuffer;
    File currentFile;
};

#endif
#endif