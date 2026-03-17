#pragma once

#include <Arduino.h>

#define ANIM_PLAY_ON_START_FLAG 0x01
#define ANIM_LOOP_ON_START_FLAG 0x02
#define ANIM_IDLE_FLAG 0x04
#define ANIM_LOOPING_FLAG 0x08
#define ANIM_PLAY_ON_PIN_LOW_FLAG 0x10
#define ANIM_PLAY_ON_PIN_HIGH_FLAG 0x20
#define ANIM_PLAY_ON_PIN_ANALOG_FLAG 0x40

#define ANIM_PLAY_ON_START(flags) ((flags & ANIM_PLAY_ON_START_FLAG) > 0)
#define ANIM_LOOP_ON_START(flags) ((flags & ANIM_LOOP_ON_START_FLAG) > 0)
#define ANIM_IS_IDLE_ANIM(flags) ((flags & ANIM_IDLE_FLAG) > 0)
#define ANIM_IS_LOOPING(flags) ((flags & ANIM_LOOPING_FLAG) > 0)
#define ANIM_PLAY_ON_PIN_LOW(flags) ((flags & ANIM_PLAY_ON_PIN_LOW_FLAG) > 0)
#define ANIM_PLAY_ON_PIN_HIGH(flags) ((flags & ANIM_PLAY_ON_PIN_HIGH_FLAG) > 0)
#define ANIM_PLAY_ON_PIN_ANALOG(flags) ((flags & ANIM_PLAY_ON_PIN_ANALOG_FLAG) > 0)

struct AnimationConfiguration
{
public:
	uint8_t flags = 0;				// Flags for different options, see ANIM_* defines and macros for details
	//uint8_t playOnStart = 0;      // play on start (0 = false) (1 = true)
	//uint8_t loopOnStart = 0;      // loop when playing on start (0 = false) (1 = true)
	//uint8_t idleAnim = 0;         // Play as an idle animation when nothing else is playing (0 = false) (1 = true)
	uint8_t playOnPin = 0;			// (0 = false) (anything above 0 up to 255 will monitor the pin given)
	//uint8_t loop = 0;             // loop when playing while triggered by pins (0 = false) (1 = true)
	//uint16_t playOnPinHigh = 0;   // pin change value (0 = play on pin LOW) (1 = play on pin HIGH) (2 = play on analog read range)	
	uint16_t buttonLadderMin = 0;	// min for button ladder
	uint16_t buttonLadderMax = 0;	// max for button ladder
};