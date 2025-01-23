#include "../BottangoArduinoModules.h"

#ifdef USE_SD_CARD_COMMAND_STREAM
#ifndef SDCardCommandStreamDataSource_h
#define SDCardCommandStreamDataSource_h

#include "AbstractCommandStreamDataSource.h"
#include "../BottangoArduinoConfig.h"
#include "SDCardUtil.h"
#include "TxtBuffer.h"

class SDCardCommandStreamDataSource : public AbstractCommandStreamDataSource
{
public:
    SDCardCommandStreamDataSource();
    SDCardCommandStreamDataSource(byte index, bool shouldLoop);
    virtual void update(bool shouldLoop);
    virtual void getNextCommand(char *output, bool shouldLoop, unsigned long &msEndOfThisCommand, unsigned long &msStartOfNextCommand) override;
    virtual void reset() override;
    ~SDCardCommandStreamDataSource();
    bool isValid = false;

private:
    byte index;
    bool setup;
    bool onLoop = false;
    bool cardReadComplete = false;
    TxtBuffer<TXT_BUFFER_SIZE_SD> cardReadBuffer;
    File currentFile;
};

#endif
#endif