#include "StatusLights.h"
#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLightsOutput.h"

#define COLOR_MAX 55
#define COLOR_MIN 5

namespace StatusLights
{

	static const unsigned long CYCLE_TIME = 3000UL;
	static const unsigned long PULSE_UP = 1500UL;
	static const unsigned long PULSE_DOWN = 1500UL;
	static const unsigned long SIGNAL_QUICK_PULSE = 150UL;
	static const unsigned long OUTPUT_MIN_INTERVAL_MS = 10UL;
	// blink segments: (short) rise/fall rise/fall rise (long) fall
	static const unsigned long blinkSegs[6] = {
		300UL, // rise 1
		300UL, // fall 1
		300UL, // rise 2
		300UL, // fall 2
		300UL, // rise 3
		1500UL // fall 3
	};

	static Color leds[NUM_STATUS_LED];
	static Color lastOutputColors[NUM_STATUS_LED];
	static bool hasOutputColors = false;
	static unsigned long lastOutputTime = 0;
	static unsigned long patternStartTime = 0;
	static LightMode lightModes[NUM_STATUS_LED];

	static Color desiredColor_Connection = STATUS_COLOR_BLACK;
	static Color desiredColor_Signal = STATUS_COLOR_BLACK;
	static Color desiredColor_User = STATUS_COLOR_BLACK;
	static Color desiredColor_Pwr = STATUS_COLOR_BLACK;

	// one-off pulse state for SIGNAL_STATUS_LIGHT
	unsigned long signalPulseStartTime = 0;
	bool isSignalPulsing = false;

	// color calculating utils
	static byte scaleChannel(byte channel, byte scale)
	{
		return ((uint16_t)channel * ((uint16_t)scale + 1)) >> 8;
	}
	static Color scaleColor(Color color, byte scale)
	{
		return Color(scaleChannel(color.r, scale), scaleChannel(color.g, scale), scaleChannel(color.b, scale));
	}
	static byte lerpChannel(byte from, byte to, byte amountOfTo)
	{
		if (to > from)
		{
			return from + scaleChannel(to - from, amountOfTo);
		}
		return from - scaleChannel(from - to, amountOfTo);
	}
	static Color lerpColor(Color from, Color to, byte amountOfTo)
	{
		return Color(
			lerpChannel(from.r, to.r, amountOfTo),
			lerpChannel(from.g, to.g, amountOfTo),
			lerpChannel(from.b, to.b, amountOfTo));
	}

	static bool outputChanged()
	{
		if (!hasOutputColors)
		{
			return true;
		}

		for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
		{
			if (leds[ledIndex].r != lastOutputColors[ledIndex].r ||
				leds[ledIndex].g != lastOutputColors[ledIndex].g ||
				leds[ledIndex].b != lastOutputColors[ledIndex].b)
			{
				return true;
			}
		}
		return false;
	}

	static void writeOutputIfNeeded(unsigned long currentMillis)
	{
		if (!outputChanged() || (hasOutputColors && (currentMillis - lastOutputTime) < OUTPUT_MIN_INTERVAL_MS))
		{
			return;
		}

		if (!StatusLightsOutput::write(leds, NUM_STATUS_LED))
		{
			return;
		}

		for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
		{
			lastOutputColors[ledIndex] = leds[ledIndex];
		}
		hasOutputColors = true;
		lastOutputTime = currentMillis;
	}

	void initLights()
	{
		StatusLightsOutput::init();

		setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);
		for (int i = 0; i < NUM_STATUS_LED; i++)
		{
			lightModes[i] = MODE_PULSE;
		}

		patternStartTime = millis();
	}

	void setDesiredColor(int light, Color color)
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
			Color baseColor;
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
				baseColor = STATUS_COLOR_BLACK;
				break;
			}

			// 2) If signal‐LED quick pulse is active, override everything
			if (ledIndex == SIGNAL_STATUS_LIGHT && isSignalPulsing)
			{
				unsigned long elapsedSinceSignalPulse = currentMillis - signalPulseStartTime;
				if (elapsedSinceSignalPulse < SIGNAL_QUICK_PULSE)
				{
					byte lerpFactor = map(elapsedSinceSignalPulse, 0, SIGNAL_QUICK_PULSE, 255, 0);

					Color dimWhite = scaleColor(Color(255, 255, 255), COLOR_MAX);
					Color targetColor = scaleColor(baseColor, COLOR_MAX);

					leds[ledIndex] = lerpColor(dimWhite, targetColor, lerpFactor);
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
			leds[ledIndex] = scaleColor(baseColor, brightnessLevel);
		}

		writeOutputIfNeeded(currentMillis);
	}

	void pulseSignalLight()
	{
		signalPulseStartTime = millis();
		isSignalPulsing = true;
	}

} // namespace StatusLights

#endif // ENABLE_STATUS_LIGHTS
