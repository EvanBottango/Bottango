#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef ENABLE_STATUS_LIGHTS

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Services/ISchedulable.h"
#include "../Services/ServiceFactory.h"
#include "../Effector/Color.h"
#include "StatusLightsOutput.h"

/**
 * @brief Manages the status lights on the device, allowing for setting desired colors and light modes (e.g. pulsing or blinking) for each light.
 * The actual displayed color of each light is determined by its desired base color modified by its current light mode and any active one-off pulse effects.
 */

class  StatusLights : public ISchedulable
{
public:
	static constexpr Color STATUS_COLOR_GREEN = Color(7, 83, 0);
	static constexpr Color STATUS_COLOR_YELLOW = Color(137, 93, 0);
	static constexpr Color STATUS_COLOR_BLUE = Color(0, 95, 108);
	static constexpr Color STATUS_COLOR_PURPLE = Color(76, 2, 113);
	static constexpr Color STATUS_COLOR_RED = Color(150, 0, 0);
	static constexpr Color STATUS_COLOR_BLACK = Color(0, 0, 0);

	static constexpr Color STATUS_COLOR_PWR_ON = STATUS_COLOR_GREEN;

	static constexpr Color STATUS_COLOR_NO_CONNECTION_SERIAL = STATUS_COLOR_YELLOW;
	static constexpr Color STATUS_COLOR_NO_CONNECTION_PEER = STATUS_COLOR_BLUE;
	static constexpr Color STATUS_COLOR_HAS_CONNECTION = STATUS_COLOR_GREEN;

	static constexpr Color STATUS_COLOR_CONNECTION_EXPORT_PLAYBACK = STATUS_COLOR_PURPLE;

	static constexpr Color STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR = STATUS_COLOR_RED;
	static constexpr Color STATUS_COLOR_SIGNAL_OFFLINEREADY = STATUS_COLOR_BLACK;
	static constexpr Color STATUS_COLOR_SIGNAL_OFFLINEPLAY = STATUS_COLOR_PURPLE;

	enum LightMode
	{
		MODE_PULSE,
		MODE_BLINK
	};

	void init() override;
	void onPhase(Phase const p) override;

	void set(int light, Color color, LightMode mode);

	/**
	 * @brief Sets the desired base color for a given status light. The actual displayed color may be modified by active light modes (e.g. pulsing or blinking) or one-off pulses.
	 * @param light The index of the light to set the color for (0-3).
	 * @param color The desired base color for the specified light.
	 */
	void setDesiredColor(int light, Color color);

	/**
	 * @brief Sets the light mode for a given status light. Light modes (e.g. pulsing or blinking) will modify the displayed color of the light over time based on the desired base color.
	 * @param light The index of the light to set the mode for (0-3).
	 * @param mode The light mode to apply to the specified light.
	 */
	void setLightMode(int light, LightMode mode);

	/**
	 * @brief Triggers a one-off quick pulse on the SIGNAL_STATUS_LIGHT. This will temporarily override the light's current mode and desired color to display a quick pulse effect, then return to the previous state.
	 */
	void pulseSignalLight();

private:
	static constexpr unsigned long CYCLE_TIME = 3000UL;
	static constexpr unsigned long PULSE_UP = 1500UL;
	static constexpr unsigned long PULSE_DOWN = 1500UL;
	static constexpr unsigned long SIGNAL_QUICK_PULSE = 150UL;
	static constexpr uint8_t COLOR_MAX = 55;
	static constexpr uint8_t COLOR_MIN = 5;
	static constexpr unsigned long OUTPUT_MIN_INTERVAL_MS = 10UL;

	// blink segments: (short) rise/fall rise/fall rise (long) fall
	static constexpr unsigned long BLINK_SEQUENCE[6] = {
		300UL, // rise 1
		300UL, // fall 1
		300UL, // rise 2
		300UL, // fall 2
		300UL, // rise 3
		1500UL // fall 3
	};

	Color m_leds[NUM_STATUS_LED];
	Color m_lastOutputColors[NUM_STATUS_LED];
	bool m_hasOutputColors = false;
	unsigned long m_lastOutputTime = 0;
	unsigned long m_patternStartTime = 0;
	LightMode m_lightModes[NUM_STATUS_LED];

	Color m_desiredColor_Connection = STATUS_COLOR_BLACK;
	Color m_desiredColor_Signal = STATUS_COLOR_BLACK;
	Color m_desiredColor_User = STATUS_COLOR_BLACK;
	Color m_desiredColor_Pwr = STATUS_COLOR_BLACK;

	// one-off pulse state for SIGNAL_STATUS_LIGHT
	unsigned long m_signalPulseStartTime = 0;
	unsigned long m_lastCall = 0;
	bool m_isSignalPulsing = false;

	void updateLights();
	uint8_t scaleChannel(uint8_t channel, uint8_t scale);
	Color scaleColor(Color color, uint8_t scale);
	uint8_t lerpChannel(uint8_t from, uint8_t to, uint8_t amountOfTo);
	Color lerpColor(Color from, Color to, uint8_t amountOfTo);
	bool outputChanged();
	void writeOutputIfNeeded(unsigned long currentMillis);
};

#endif