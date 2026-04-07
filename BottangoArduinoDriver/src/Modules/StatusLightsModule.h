#include "../../BottangoArduinoModules.h"

#ifdef ENABLE_STATUS_LIGHTS

#ifndef _StatusLightsModule_h
#define _StatusLightsModule_h

#define STATUS_COLOR_GREEN CRGB(7, 83, 0)
#define STATUS_COLOR_YELLOW CRGB(137, 93, 0)
#define STATUS_COLOR_BLUE CRGB(0, 95, 108)
#define STATUS_COLOR_PURPLE CRGB(76, 2, 113)
#define STATUS_COLOR_RED CRGB(150, 0, 0)
#define STATUS_COLOR_BLACK CRGB(0, 0, 0)

#define STATUS_COLOR_PWR_ON STATUS_COLOR_GREEN

#define STATUS_COLOR_NO_CONNECTION_SERIAL STATUS_COLOR_YELLOW
#define STATUS_COLOR_NO_CONNECTION_PEER STATUS_COLOR_BLUE
#define STATUS_COLOR_HAS_CONNECTION STATUS_COLOR_GREEN

#define STATUS_COLOR_CONNECTION_EXPORT_PLAYBACK STATUS_COLOR_PURPLE

#define STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR STATUS_COLOR_RED
#define STATUS_COLOR_SIGNAL_OFFLINEREADY STATUS_COLOR_BLACK
#define STATUS_COLOR_SIGNAL_OFFLINEPLAY STATUS_COLOR_PURPLE

#define COLOR_MAX 55
#define COLOR_MIN 5
#define PULSE_TIME B_ON_SHORT // use the short blink duration for the one-off pulse

#include <Arduino.h>
#include <FastLED.h>
#include "../Module Handling/ModuleMaster.h"
#include "../../BottangoArduinoConfig.h"

class StatusLightsModule : public LoopModule
{
public:
	void onPhase(Phase p) override;
	void init() override;

private:
	enum LightMode
	{
		MODE_PULSE,
		MODE_BLINK
	};

	static const unsigned long CYCLE_TIME = 3000UL;
	static const unsigned long PULSE_UP = 1500UL;
	static const unsigned long PULSE_DOWN = 1500UL;
	static const unsigned long SIGNAL_QUICK_PULSE = 150UL;

	// blink segments: (short) rise/fall rise/fall rise (long) fall
	static constexpr unsigned long blinkSegs[6] = {
		300UL, // rise 1
		300UL, // fall 1
		300UL, // rise 2
		300UL, // fall 2
		300UL, // rise 3
		1500UL // fall 3
	};

	CRGB leds[NUM_STATUS_LED];
	unsigned long patternStartTime = 0;
	LightMode lightModes[NUM_STATUS_LED];

	CRGB desiredColor_Connection = CRGB::Black;
	CRGB desiredColor_Signal = CRGB::Black;
	CRGB desiredColor_User = CRGB::Black;
	CRGB desiredColor_Pwr = CRGB::Black;

	// one-off pulse state for SIGNAL_STATUS_LIGHT
	unsigned long signalPulseStartTime = 0;
	bool isSignalPulsing = false;

	
	void updateLights();
	void setDesiredColor(int light, CRGB color);
	void setLightMode(int light, LightMode mode);
	void resetPreferencesAnimation();
	//void pulseSignalLight();

};

#endif
#endif