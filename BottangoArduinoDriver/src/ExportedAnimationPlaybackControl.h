#ifndef EXPORTEDANIMATIONPLAYBACKCONTROL_H
#define EXPORTEDANIMATIONPLAYBACKCONTROL_H

#include "../BottangoArduinoConfig.h"
#include "../BottangoArduinoModules.h"
#include "CircularArray.h"

#ifdef USE_SD_CARD_COMMAND_STREAM
#include "SD.h"
#endif

#ifdef USE_CODE_COMMAND_STREAM
#include "../GeneratedCodeAnimations.h"
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
namespace ExportedAnimationPlaybackControl
{
    class AnimationConfiguration
    {
    public:
        uint8_t playOnStart = 0;      // play on start (0 == false) (1 == true)
        uint8_t loopOnStart = 0;      // loop when playing on start (0 == false) (1 == true)
        uint8_t idleAnim = 0;         // Play as an idle animation when nothing else is playing (0 == false) (1 == true)
        uint8_t playOnPin = 0;        // (0 == false) (anything above 0 up to 255 will monitor the pin given)
        uint8_t loop = 0;             // loop when playing while triggered by pins (0 == false) (1 == true)
        uint16_t playOnPinHigh = 0;   // pin change value (0 == play on pin LOW) (1 == play on pin HIGH) (2 == play on analog read range)
        uint16_t buttonLadderMin = 0; // min for button ladder
        uint16_t buttonLadderMax = 0; // max for button ladder
    };

    extern CircularArray<AnimationConfiguration> animationConfigs;

    void initialize();
    void updatePlaybackStatus();
    int getIndexOfAnimationToTrigger(bool interruptingAnimationsOnly);
    extern int currentPlayingIndex;
    extern int defaultIndex;

#ifdef USE_SD_CARD_COMMAND_STREAM
    AnimationConfiguration *parseConfiguration(File configFile);
#elif defined(USE_CODE_COMMAND_STREAM)
    AnimationConfiguration *parseConfiguration(const uint16_t *configValues);
#endif

#ifdef EXPORTED_ANIM_LOGGING
    void logConfig(AnimationConfiguration *config);
#endif
}
#endif

#endif