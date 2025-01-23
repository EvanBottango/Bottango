#ifndef CommandStreamProvider_h
#define CommandStreamProvider_h

#include "CommandStream.h"
#include "Arduino.h"
#include "../BottangoArduinoModules.h"

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#include "ExportedAnimationPlaybackControl.h"
#endif

#ifdef USE_CODE_COMMAND_STREAM
#include "CodeCommandStreamDataSource.h"
#include "../GeneratedCodeAnimations.h"
#endif

#ifdef USE_SD_CARD_COMMAND_STREAM
#include "SDCardCommandStreamDataSource.h"
#endif

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
    CommandStream *commandStream = nullptr;
    void runInProgressCommand();
};
#endif