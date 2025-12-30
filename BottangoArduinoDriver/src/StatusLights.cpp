#include "../BottangoArduinoModules.h"
#ifdef ENABLE_STATUS_LIGHTS_OLD
#include "StatusLights.h"

#define COLOR_MAX 55
#define COLOR_MIN 5
#define PULSE_TIME B_ON_SHORT // use the short blink duration for the one-off pulse

namespace StatusLights
{

    static const unsigned long CYCLE_TIME = 3000UL;
    static const unsigned long PULSE_UP = 1500UL;
    static const unsigned long PULSE_DOWN = 1500UL;
    static const unsigned long SIGNAL_QUICK_PULSE = 150UL;
    // blink segments: (short) rise/fall rise/fall rise (long) fall
    static const unsigned long blinkSegs[6] = {
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

    void initLights()
    {
        FastLED.addLeds<WS2812B, STATUS_LED_PIN, GRB>(leds, NUM_STATUS_LED);
        FastLED.clear();
        FastLED.show();

        setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);
        for (int i = 0; i < NUM_STATUS_LED; i++)
        {
            lightModes[i] = MODE_PULSE;
        }

        patternStartTime = millis();
        FastLED.show();
    }

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
        }
    }

    void setLightMode(int light, LightMode mode)
    {
        if (light >= 0 && light < NUM_STATUS_LED)
            lightModes[light] = mode;
    }

    void updateLights()
    {
        unsigned long currentMillis = millis();
        unsigned long elapsedPatternTime = (currentMillis - patternStartTime) % CYCLE_TIME;

        for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
        {
            // 1) Determine the base color for this LED
            CRGB baseColor;
            switch (ledIndex)
            {
            case CONNECTION_STATUS_LIGHT:
                baseColor = desiredColor_Connection;
                break;
            case SIGNAL_STATUS_LIGHT:
                baseColor = desiredColor_Signal;
                break;
            case USER_STATUS_LIGHT:
                baseColor = desiredColor_User;
                break;
            case PWR_STATUS_LIGHT:
                baseColor = desiredColor_Pwr;
                break;
            default:
                baseColor = CRGB::Black;
                break;
            }

            // 2) If signal‐LED quick pulse is active, override everything
            if (ledIndex == SIGNAL_STATUS_LIGHT && isSignalPulsing)
            {
                unsigned long elapsedSinceSignalPulse = currentMillis - signalPulseStartTime;
                if (elapsedSinceSignalPulse < SIGNAL_QUICK_PULSE)
                {
                    int lerpFactor = map(elapsedSinceSignalPulse, 0, SIGNAL_QUICK_PULSE, 255, 0);

                    CRGB halfWhite = CRGB::White;
                    halfWhite.nscale8(55);

                    CRGB targetColor = baseColor;
                    targetColor.nscale8(COLOR_MAX);

                    leds[ledIndex] = halfWhite.lerp8(targetColor, lerpFactor);
                    continue;
                }
                else
                {
                    isSignalPulsing = false;
                }
            }

            // 3) Compute brightness level for pulse or blink
            int brightnessLevel = COLOR_MIN;

            if (lightModes[ledIndex] == MODE_PULSE)
            {
                // Smooth pulse up/down over CYCLE_TIME
                if (elapsedPatternTime < PULSE_UP)
                {
                    brightnessLevel = map(elapsedPatternTime, 0, PULSE_UP, COLOR_MIN, COLOR_MAX);
                }
                else
                {
                    unsigned long elapsedSincePeak = elapsedPatternTime - PULSE_UP;
                    brightnessLevel = map(elapsedSincePeak, 0, PULSE_DOWN, COLOR_MAX, COLOR_MIN);
                }
            }
            else // MODE_BLINK
            {
                // Find which blink segment we're in
                int blinkSegmentIndex = 0;
                unsigned long cumulativeDuration = 0;

                for (int i = 0; i < 6; i++)
                {
                    cumulativeDuration += blinkSegs[i];
                    if (elapsedPatternTime < cumulativeDuration)
                    {
                        blinkSegmentIndex = i;
                        break;
                    }
                }

                unsigned long segmentStartTime = cumulativeDuration - blinkSegs[blinkSegmentIndex];
                unsigned long elapsedInSegment = elapsedPatternTime - segmentStartTime;
                unsigned long currentSegmentLength = blinkSegs[blinkSegmentIndex];

                bool isRisingSegment = (blinkSegmentIndex % 2 == 0);
                if (isRisingSegment)
                {
                    brightnessLevel = map(elapsedInSegment, 0, currentSegmentLength, COLOR_MIN, COLOR_MAX);
                }
                else
                {
                    brightnessLevel = map(elapsedInSegment, 0, currentSegmentLength, COLOR_MAX, COLOR_MIN);
                }
            }

            // 4) Scale the base color and assign to the LED
            CRGB scaledColor = baseColor;
            scaledColor.nscale8(brightnessLevel);
            leds[ledIndex] = scaledColor;
        }

        FastLED.show();
    }

    void pulseSignalLight()
    {
        signalPulseStartTime = millis();
        isSignalPulsing = true;
    }

} // namespace StatusLights

#endif // ENABLE_STATUS_LIGHTS