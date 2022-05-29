#ifndef CommandStreamProvider_h
#define CommandStreamProvider_h

#include "CommandStream.h"
#include "Arduino.h"

class CommandStreamProvider
{
public:
    CommandStreamProvider();

    void runSetup();
    void startCommandStream(byte streamID, bool loop);
    void updateOnLoop();
    bool streamIsInProgress();

    void stop();

protected:
    CommandStream *streamInProgress = NULL;
    void runInProgressCommand();
};
#endif