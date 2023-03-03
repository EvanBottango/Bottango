#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "BasicCommands.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
#include "CommandStreamProvider.h"
#include <Arduino.h>

namespace BottangoCore
{
    void bottangoSetup();

    void bottangoLoop();

    void stop();

    void executeCommand(char *commandString);

    unsigned long getMSTimeOfCommand(char *commandString);

    bool externalCommandIsValid(char *commandString);

    extern EffectorPool effectorPool;
    extern bool initialized;
#ifdef USE_COMMAND_STREAM
    extern CommandStreamProvider commandStreamProvider;
#endif

} // namespace BottangoCore

#endif // BOTTANGO_CORE_H
