// OfflineAnimationControl.h

#ifndef _OfflineAnimationControl_h
#define _OfflineAnimationControl_h

#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include <Arduino.h>
#include "../Module Handling/ModuleLoop.h"
#include "../CircularArray.h"

class AnimationPlaybackControl : LoopModule
{
public:
	struct AnimationConfiguration
	{
	public:
		uint_fast8_t playOnStart = 0;      // play on start (0 == false) (1 == true)
		uint_fast8_t loopOnStart = 0;      // loop when playing on start (0 == false) (1 == true)
		uint_fast8_t idleAnim = 0;         // Play as an idle animation when nothing else is playing (0 == false) (1 == true)
		uint_fast8_t playOnPin = 0;        // (0 == false) (anything above 0 up to 255 will monitor the pin given)
		uint_fast8_t loop = 0;             // loop when playing while triggered by pins (0 == false) (1 == true)
		uint_fast16_t playOnPinHigh = 0;   // pin change value (0 == play on pin LOW) (1 == play on pin HIGH) (2 == play on analog read range)
		uint_fast16_t buttonLadderMin = 0; // min for button ladder
		uint_fast16_t buttonLadderMax = 0; // max for button ladder
	};

	void onPhase(Phase p) override;

	void init() override {};

	void runSetup();

private:
	CircularArray<AnimationConfiguration> animationConfigs = CircularArray<AnimationConfiguration>(MAX_EXPORTED_ANIMATIONS);
	int currentPlayingIndex = -1;
	int idleAnimIndex = -1;
	int startingAnim = -1;
};

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
#endif // _OfflineAnimationControl_h