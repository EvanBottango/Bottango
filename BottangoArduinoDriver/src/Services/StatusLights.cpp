#include "StatusLights.h"
#ifdef ENABLE_STATUS_LIGHTS

void StatusLights::init()
{
	StatusLightsOutput::init();

	setDesiredColor(PWR_STATUS_LIGHT, STATUS_COLOR_PWR_ON);
	for (int i = 0; i < NUM_STATUS_LED; i++)
	{
		m_lightModes[i] = MODE_PULSE;
	}

	m_patternStartTime = millis();
}

void StatusLights::onPhase(Phase const p)
{
	if (p != Phase::Output) return;

	updateLights();
}

void StatusLights::set(int light, Color color, LightMode mode)
{
	setDesiredColor(light, color);
	setLightMode(light, mode);
}

void StatusLights::setDesiredColor(int light, Color color)
{
	switch (light)
	{
	case CONNECTION_STATUS_LIGHT:
		m_desiredColor_Connection = color;
		break;
	case SIGNAL_STATUS_LIGHT:
		m_desiredColor_Signal = color;
		break;
	case USER_STATUS_LIGHT:
		m_desiredColor_User = color;
		break;
	case PWR_STATUS_LIGHT:
		m_desiredColor_Pwr = color;
		break;
	}
}

void StatusLights::setLightMode(int light, LightMode mode)
{
	if (light >= 0 && light < NUM_STATUS_LED)
	{
		m_lightModes[light] = mode;
	}
}

void StatusLights::updateLights()
{
	unsigned long currentMillis = millis();
	unsigned long elapsedPatternTime = (currentMillis - m_patternStartTime) % CYCLE_TIME;

	for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
	{
		// 1) Determine the base color for this LED
		Color baseColor;
		switch (ledIndex)
		{
		case CONNECTION_STATUS_LIGHT:
			baseColor = m_desiredColor_Connection;
			break;
		case SIGNAL_STATUS_LIGHT:
			baseColor = m_desiredColor_Signal;
			break;
		case USER_STATUS_LIGHT:
			baseColor = m_desiredColor_User;
			break;
		case PWR_STATUS_LIGHT:
			baseColor = m_desiredColor_Pwr;
			break;
		default:
			baseColor = STATUS_COLOR_BLACK;
			break;
		}

		// 2) If signal‐LED quick pulse is active, override everything
		if (ledIndex == SIGNAL_STATUS_LIGHT && m_isSignalPulsing)
		{
			unsigned long elapsedSinceSignalPulse = currentMillis - m_signalPulseStartTime;
			if (elapsedSinceSignalPulse < SIGNAL_QUICK_PULSE)
			{
				int lerpFactor = map(elapsedSinceSignalPulse, 0, SIGNAL_QUICK_PULSE, 255, 0);

				Color dimWhite = scaleColor(Color(255, 255, 255), COLOR_MAX);
				Color targetColor = scaleColor(baseColor, COLOR_MAX);

				m_leds[ledIndex] = lerpColor(dimWhite, targetColor, lerpFactor);
				continue;
			}
			else
			{
				m_isSignalPulsing = false;
			}
		}

		// 3) Compute brightness level for pulse or blink
		int brightnessLevel = COLOR_MIN;

		if (m_lightModes[ledIndex] == MODE_PULSE)
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
				cumulativeDuration += BLINK_SEQUENCE[i];
				if (elapsedPatternTime < cumulativeDuration)
				{
					blinkSegmentIndex = i;
					break;
				}
			}

			unsigned long segmentStartTime = cumulativeDuration - BLINK_SEQUENCE[blinkSegmentIndex];
			unsigned long elapsedInSegment = elapsedPatternTime - segmentStartTime;
			unsigned long currentSegmentLength = BLINK_SEQUENCE[blinkSegmentIndex];

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
		m_leds[ledIndex] = scaleColor(baseColor, brightnessLevel);
	}

	writeOutputIfNeeded(currentMillis);
}

void StatusLights::pulseSignalLight()
{
	m_signalPulseStartTime = millis();
	m_isSignalPulsing = true;
}

uint8_t StatusLights::scaleChannel(uint8_t channel, uint8_t scale)
{
	return (static_cast<uint16_t>(channel) * (static_cast<uint16_t>(scale) + 1)) >> 8;
}

Color StatusLights::scaleColor(Color color, uint8_t scale)
{
	return Color(scaleChannel(color.r, scale), scaleChannel(color.g, scale), scaleChannel(color.b, scale));
}

uint8_t StatusLights::lerpChannel(uint8_t from, uint8_t to, uint8_t amountOfTo)
{
	if (to > from)
	{
		return from + scaleChannel(to - from, amountOfTo);
	}
	return from - scaleChannel(from - to, amountOfTo);
}

Color StatusLights::lerpColor(Color from, Color to, uint8_t amountOfTo)
{
	return Color(
		lerpChannel(from.r, to.r, amountOfTo),
		lerpChannel(from.g, to.g, amountOfTo),
		lerpChannel(from.b, to.b, amountOfTo));
}

bool StatusLights::outputChanged()
{
	if (!m_hasOutputColors)
	{
		return true;
	}

	for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
	{
		if (m_leds[ledIndex].r != m_lastOutputColors[ledIndex].r ||
			m_leds[ledIndex].g != m_lastOutputColors[ledIndex].g ||
			m_leds[ledIndex].b != m_lastOutputColors[ledIndex].b)
		{
			return true;
		}
	}
	return false;
}

void StatusLights::writeOutputIfNeeded(unsigned long currentMillis)
{
	if (!outputChanged() || (m_hasOutputColors && (currentMillis - m_lastOutputTime) < OUTPUT_MIN_INTERVAL_MS))
	{
		return;
	}

	if (!StatusLightsOutput::write(m_leds, NUM_STATUS_LED))
	{
		return;
	}

	for (int ledIndex = 0; ledIndex < NUM_STATUS_LED; ++ledIndex)
	{
		m_lastOutputColors[ledIndex] = m_leds[ledIndex];
	}
	m_hasOutputColors = true;
	m_lastOutputTime = currentMillis;
}

#endif // ENABLE_STATUS_LIGHTS