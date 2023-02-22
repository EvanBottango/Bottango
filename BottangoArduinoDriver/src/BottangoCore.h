#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "CommandRegistry.h"
#include "BasicCommands.h"
#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoCallbacks.h"
#include "CommandStreamProvider.h"

namespace BottangoCore
{
    void bottangoSetup();

    void bottangoLoop();

    void stop();

    extern CommandRegistry commandRegistry;
    extern EffectorPool effectorPool;
    extern CommandStreamProvider commandStreamProvider;
    extern bool initialized;
} // namespace BottangoCore

#endif // BOTTANGO_CORE_H
