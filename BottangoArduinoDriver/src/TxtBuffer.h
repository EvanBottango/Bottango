#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW) || defined(USE_SD_CARD_COMMAND_STREAM)

#ifndef TxtBuffer_h
#define TxtBuffer_h

#include "../BottangoArduinoConfig.h"

template <int BufferSize>
class TxtBuffer
{
public:
    TxtBuffer();
    void addTxt(const char *txt);
    void addChar(const char c);
    bool available();
    int getSpaceAvailable();
    int getSpaceUsed();
    void getNextTxt(char *outTxt, bool peek = false);
    bool isFull();
    void clear();

private:
    int head = 0;
    int tail = 0;
    char buffer[BufferSize];
};
#include "TxtBuffer-inl.h"

#endif
#endif