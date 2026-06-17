#pragma once

// enables quick modules settings for Bottango boards
#include <Arduino.h>

// #define BOTTANGO_IMPULSE
// #define BOTTANGO_NOVA
// #define BOTTANGO_SOLAR

#if defined(BOTTANGO_IMPULSE) || defined(BOTTANGO_NOVA) || defined(BOTTANGO_SOLAR)
#define OVERRIDE_MODULES
#endif

#ifdef OVERRIDE_MODULES
// regular comms
#define USE_USB_SERIAL

// uid
#define REPORT_UID

// sync commands
#define ALLOW_SYNC_COMMANDS

// status lights
#define ENABLE_STATUS_LIGHTS

// ota supported
#define ENABLE_ESP_OTA_UPDATE

// relay
#define RELAY_SUPPORTED
#define RELAY_COMS_ESPNOW
// #define RELAY_COMS_RS485

// Utility pin
#define UTILITY_PIN 0

// reset prefs
#define RESET_PREFS_SUPPORTED

// toggle extra logging
#define TOGGLE_DEBUG
#define TOGGLE_DEBUG_PRESS_COUNT 3

// named board (solar, nova, etc.)
#define NAMED_BOARD

// special startup/loop
#define NAMED_BOARD_STARTUP
#define EN_PIN_ON_STARTUP
#define BOARD_EN_PIN 2
#define NAMED_BOARD_LOOP

// buttons actions
#define STOP_BUTTON_SUPPORTED
#define DYNAMIC_STOP_BUTTON_BEHAVIOR
#endif

#ifdef BOTTANGO_SOLAR
#define USE_SD_CARD_COMMAND_STREAM
#define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#define AUDIO_SD_I2S
#define DYNAMIC_VOLUME
#define PIN_REMAPPING
#define PIN_LOW_LAUNCH
const char NAMED_BOARD_MODEL[] PROGMEM = "SOLAR";
#define ONLINE_BUTTON_ACTIONS
#define NAMED_BUTTON_A_PIN 39
#define NAMED_BUTTON_B_PIN 39
#define NAMED_BUTTON_C_PIN 39
#define NAMED_BUTTON_A_READ_MIN 2350
#define NAMED_BUTTON_A_READ_MAX 3650
#define NAMED_BUTTON_B_READ_MIN 1250
#define NAMED_BUTTON_B_READ_MAX 2250
#define NAMED_BUTTON_C_READ_MIN 550
#define NAMED_BUTTON_C_READ_MAX 1150
#define NAMED_BOARD_LOOP
#define NAMED_BOARD_BUTTON_DEBOUNCE 250
#endif

#ifdef BOTTANGO_IMPULSE
#define PIN_REMAPPING
#define PIN_LOW_LAUNCH
const char NAMED_BOARD_MODEL[] PROGMEM = "IMPULSE";
#endif

#ifdef BOTTANGO_NOVA
#define USE_SD_CARD_COMMAND_STREAM
#define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
const char NAMED_BOARD_MODEL[] PROGMEM = "NOVA";
#define ONLINE_BUTTON_ACTIONS
#define NAMED_BUTTON_A_PIN 34
#define NAMED_BUTTON_B_PIN 35
#define NAMED_BUTTON_C_PIN 36
#define NAMED_BOARD_LOOP
#define NAMED_BOARD_BUTTON_DEBOUNCE 250
#endif

#ifdef NAMED_BOARD_STARTUP
namespace NamedBoardStartup
{
	// keep this simple and in-line-able
	inline void runNamedBoardStartup()
	{
#ifdef EN_PIN_ON_STARTUP
		pinMode(BOARD_EN_PIN, OUTPUT);
		digitalWrite(BOARD_EN_PIN, HIGH);
#endif

#ifdef ONLINE_BUTTON_ACTIONS
		pinMode(NAMED_BUTTON_A_PIN, INPUT);
		pinMode(NAMED_BUTTON_B_PIN, INPUT);
		pinMode(NAMED_BUTTON_C_PIN, INPUT);
#endif
	}
}
#endif

#ifdef NAMED_BOARD_LOOP
namespace NamedBoardLoop
{
	void runNamedBoardLoop();
}

#endif

#ifdef TOGGLE_DEBUG
namespace NamedBoardDebugToggle
{
	bool debugEnabled();
	void onDebugToggled();
}
#endif