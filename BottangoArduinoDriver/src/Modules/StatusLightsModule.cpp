#include "../../BottangoArduinoModules.h"

#ifdef ENABLE_STATUS_LIGHTS

#include "StatusLightsModule.h"
#include "../System/SystemStatus.h"


void StatusLightsModule::onPhase(Phase p)
{
	if (p != Phase::Output) return;

	switch (SystemStatus::systemStatus.ConnectionStatus)
	{
	case SystemStatus::eConnectionStatus::Off:
		desiredColor_Connection = STATUS_COLOR_BLACK;
		break;
	case SystemStatus::eConnectionStatus::Has_Connection:
		desiredColor_Connection = STATUS_COLOR_HAS_CONNECTION;
		setLightMode(CONNECTION_STATUS_LIGHT, LightMode::MODE_PULSE);
		break;
	case SystemStatus::eConnectionStatus::No_Connection_Peer:
		desiredColor_Connection = STATUS_COLOR_NO_CONNECTION_PEER;
		setLightMode(CONNECTION_STATUS_LIGHT, LightMode::MODE_BLINK);
		break;
	case SystemStatus::eConnectionStatus::No_Connection_Serial:
		desiredColor_Connection = STATUS_COLOR_NO_CONNECTION_SERIAL;
		setLightMode(CONNECTION_STATUS_LIGHT, LightMode::MODE_BLINK);
		break;
	case SystemStatus::eConnectionStatus::Export_Playback:
		desiredColor_Connection = STATUS_COLOR_CONNECTION_EXPORT_PLAYBACK;
		setLightMode(CONNECTION_STATUS_LIGHT, LightMode::MODE_PULSE);
		break;
	case SystemStatus::eConnectionStatus::Red:
		desiredColor_Connection = STATUS_COLOR_RED;
		break;
	default:
		break;
	}

	switch (SystemStatus::systemStatus.Signal)
	{
	case SystemStatus::eSignal::Off:
		desiredColor_Signal = STATUS_COLOR_BLACK;
		break;
	case SystemStatus::eSignal::OfflinePlayback:
		desiredColor_Signal = STATUS_COLOR_SIGNAL_OFFLINEPLAY;
		break;
	case SystemStatus::eSignal::OfflineReady:
		desiredColor_Signal = STATUS_COLOR_SIGNAL_OFFLINEREADY;
		break;
	default:
		break;
	}

	if (SystemStatus::systemStatus.CommandStatus == SystemStatus::eCommandStatus::NewCommand)
	{
		signalPulseStartTime = millis();
		isSignalPulsing = true;
		SystemStatus::systemStatus.CommandStatus = SystemStatus::eCommandStatus::Idle;
	}

	/*if (SystemStatus::systemStatus.resetPreferences)
	{

	}*/

	updateLights();
}

void StatusLightsModule::init()
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

void StatusLightsModule::setDesiredColor(int light, CRGB color)
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

void StatusLightsModule::setLightMode(int light, LightMode mode)
{
	if (light >= 0 && light < NUM_STATUS_LED)
	{
		lightModes[light] = mode;
	}
}

void StatusLightsModule::updateLights()
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

void StatusLightsModule::resetPreferencesAnimation()
{
	for (int i = 0; i < 3; i++)
	{
		setDesiredColor(CONNECTION_STATUS_LIGHT, CRGB::White * 0.33f);
		setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::White * 0.33f);
		setDesiredColor(USER_STATUS_LIGHT, CRGB::White * 0.33f);
		updateLights();
		delay(100);
		setDesiredColor(CONNECTION_STATUS_LIGHT, CRGB::Black);
		setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
		setDesiredColor(USER_STATUS_LIGHT, CRGB::Black);
		updateLights();
		delay(100);
	}
}

/*void StatusLightsModule::pulseSignalLight()
{
	signalPulseStartTime = millis();
	isSignalPulsing = true;
}*/
#endif