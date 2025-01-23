#include "../BottangoArduinoModules.h"
#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"

#define FADE_TIME 3000
#define PULSE_TIME 150
#define COLOR_MAX 55
#define COLOR_MIN 5

namespace StatusLights
{
    CRGB leds[NUM_STATUS_LED];
    int brightness = COLOR_MAX;
    unsigned long lastTime = 0;

    CRGB desiredColor_Connection = CRGB::Black;
    CRGB desiredColor_Signal = CRGB::Black;
    CRGB desiredColor_User = CRGB::Black;
    CRGB desiredColor_Pwr = CRGB::Black;

    unsigned long signalpulseStartTime = 0;
    bool signalisPulsing = false;

    void setDesiredColor(int light, CRGB color)
    {
        switch (light)
        {
        case CONNECTION_STATUS_LIGHT:
            desiredColor_Connection = color;
            break;
        case SIGNAL_STATUS_LIGHT:
            desiredColor_Signal = color;
            break;
        case USER_STATUS_LIGHT:
            desiredColor_User = color;
            break;
        case PWR_STATUS_LIGHT:
            desiredColor_Pwr = color;
            break;
        };
    }

    void initLights()
    {
        FastLED.addLeds<WS2812B, STATUS_LED_PIN, GRB>(leds, NUM_STATUS_LED);
        FastLED.clear();
        FastLED.show();

        setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);

        FastLED.show();
    }

    void updateLights()
    {
        unsigned long currentMillis = millis();
        unsigned long elapsedTime = currentMillis % FADE_TIME;

        int fadeBrightness;
        if (elapsedTime < FADE_TIME / 2)
        {
            fadeBrightness = map(elapsedTime, 0, FADE_TIME / 2, COLOR_MIN, COLOR_MAX);
        }
        else
        {
            fadeBrightness = map(elapsedTime, FADE_TIME / 2, FADE_TIME, COLOR_MAX, COLOR_MIN);
        }

        leds[CONNECTION_STATUS_LIGHT] = desiredColor_Connection;
        leds[CONNECTION_STATUS_LIGHT].nscale8(fadeBrightness);

        leds[PWR_STATUS_LIGHT] = desiredColor_Pwr;
        leds[PWR_STATUS_LIGHT].nscale8(fadeBrightness);

        leds[USER_STATUS_LIGHT] = desiredColor_User;

        // Handle pulsing for SIGNAL_STATUS_LIGHT
        if (signalisPulsing && (currentMillis - signalpulseStartTime < PULSE_TIME))
        {
            // Calculate pulse progress from 0 to 255
            int pulseProgress = map(currentMillis - signalpulseStartTime, 0, PULSE_TIME, 255, 0);

            // Blend from 1/2 white to the current fade color based on pulse progress
            leds[SIGNAL_STATUS_LIGHT] = CRGB::White;
            leds[SIGNAL_STATUS_LIGHT].nscale8(55);

            CRGB fadeColor = desiredColor_Signal;
            fadeColor.nscale8(fadeBrightness);
            leds[SIGNAL_STATUS_LIGHT] = leds[SIGNAL_STATUS_LIGHT].lerp8(fadeColor, pulseProgress);
        }
        else
        {
            signalisPulsing = false;
            leds[SIGNAL_STATUS_LIGHT] = desiredColor_Signal;
            leds[SIGNAL_STATUS_LIGHT].nscale8(fadeBrightness); // Resume fade effect
        }

        FastLED.show();
    }

    void pulseSignalLight()
    {
        signalpulseStartTime = millis();
        signalisPulsing = true;
    }

} // namespace StatusLights
#endif