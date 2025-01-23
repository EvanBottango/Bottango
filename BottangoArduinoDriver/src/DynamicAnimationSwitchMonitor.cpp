#include "DynamicAnimationSwitchMonitor.h"
#include "../BottangoArduinoConfig.h"

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
namespace DynamicAnimationSwitch
{
    bool animationSwitchInitialized = false;
    bool shouldRunCommandStreams = false;

    void init()
    {
        if (!animationSwitchInitialized)
        {
            pinMode(ANIMATION_STATE_SELECTION_PIN, INPUT);
            animationSwitchInitialized = true;
        }

#ifdef SELECT_EXPORTED_IS_LOW
        shouldRunCommandStreams = digitalRead(ANIMATION_STATE_SELECTION_PIN) == LOW;
#else
        shouldRunCommandStreams = digitalRead(ANIMATION_STATE_SELECTION_PIN) == HIGH;
#endif
    }

} // namespace  Dynamic Animation Switch

#endif