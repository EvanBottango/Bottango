#ifndef BOTTANGO_CORE_H
#define BOTTANGO_CORE_H

#include "EffectorPool.h"
#include "CommandRegistry.h"
#include "BasicCommands.h"
#include "../BottangoArduinoConfig.h"
#include "CommandStreamProvider.h"

namespace BottangoCore
{
    void bottangoSetup();

    void bottangoLoop();

    extern CommandRegistry commandRegistry;
    extern EffectorPool effectorPool;
    extern CommandStreamProvider commandStreamProvider;
    extern bool initialized;
    extern bool drivePaused;
} // namespace BottangoCore

#endif //BOTTANGO_CORE_H
