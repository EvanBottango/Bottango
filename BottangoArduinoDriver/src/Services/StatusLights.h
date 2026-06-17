#ifndef StatusLights_h
#define StatusLights_h

#include "../BottangoArduinoModules.h"

#ifdef ENABLE_STATUS_LIGHTS

#include "Arduino.h"
#include <FastLED.h>
#include "../BottangoArduinoConfig.h"

namespace StatusLights
{
    enum LightMode
    {
        MODE_PULSE,
        MODE_BLINK
    };

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

    void initLights();
    void updateLights();
    void setDesiredColor(int light, CRGB color);
    void setLightMode(int light, LightMode mode);
    void pulseSignalLight();

    extern CRGB leds[NUM_STATUS_LED];
    extern unsigned long patternStartTime;
    extern LightMode lightModes[NUM_STATUS_LED];

    extern CRGB desiredColor_Connection;
    extern CRGB desiredColor_Signal;
    extern CRGB desiredColor_User;
    extern CRGB desiredColor_Pwr;

} // namespace statusLights

#endif
#endif