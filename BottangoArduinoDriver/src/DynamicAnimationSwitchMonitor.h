#ifndef DynamicAnimationSwitchMonitor_h
#define DynamicAnimationSwitchMonitor_h

// check should compile
#include "../BottangoArduinoModules.h"
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH

#include "Arduino.h"
namespace DynamicAnimationSwitch
{
    void init();
    extern bool shouldRunCommandStreams;
}
#endif // conditional compilation monitor

#endif // namespace DynamicAnimationSwitch