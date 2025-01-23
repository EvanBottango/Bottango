#ifndef StatusLights_h
#define StatusLights_h

#include "../BottangoArduinoModules.h"

#ifdef ENABLE_STATUS_LIGHTS

#include "Arduino.h"
#include <FastLED.h>
#include "../BottangoArduinoConfig.h"

namespace StatusLights
{

#define STATUS_COLOR_PWR_ON CRGB(8, 136, 255)

#define STATUS_COLOR_NO_CONNECTION CRGB(220, 192, 0)
#define STATUS_COLOR_HAS_CONNECTION CRGB(8, 136, 255)
#define STATUS_COLOR_LOST_CONNECTION CRGB(225, 12, 0)
#define STATUS_COLOR_CONNECTION_OFFLINE CRGB(193, 3, 255)

#define STATUS_COLOR_SIGNAL_SERIAL CRGB(0, 250, 18)
#define STATUS_COLOR_SIGNAL_CHILD CRGB(3, 228, 255)

#define STATUS_COLOR_SIGNAL_NOSD CRGB(225, 12, 0)
#define STATUS_COLOR_SIGNAL_NOANIM CRGB(220, 192, 0)
#define STATUS_COLOR_SIGNAL_OFFLINEREADY CRGB(8, 136, 255)
#define STATUS_COLOR_SIGNAL_OFFLINEPLAY CRGB(0, 250, 18)

    void initLights();
    void updateLights();
    void setDesiredColor(int light, CRGB color);
    void pulseSignalLight();

    extern CRGB leds[NUM_STATUS_LED];
    extern int brightness;
    extern unsigned long lastTime;

    extern CRGB desiredColor_Connection;
    extern CRGB desiredColor_Signal;
    extern CRGB desiredColor_User;
    extern CRGB desiredColor_Pwr;

} // namespace statusLights

#endif
#endif