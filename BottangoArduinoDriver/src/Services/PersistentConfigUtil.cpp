#include "PersistentConfigUtil.h"
#include "../Communication/Outgoing.h"
#include "../BoardDefs.h"
#include "../../BottangoArduinoConfig.h"

#if defined(RELAY_SUPPORTED) || defined(REPORT_UID)
#include "../Util/UDIDHelper.h"
#endif

#if defined(RESET_PREFS_SUPPORTED) && defined(ENABLE_STATUS_LIGHTS)
#include "StatusLights.h"
#endif

#if defined(RESET_PREFS_SUPPORTED)
#include "../Communication/BasicCommands.h"
#endif

namespace PersistentConfigUtil
{

#ifdef ESP32
	Preferences prefs;

	void begin()
	{
		if (!prefs.begin(ESP32_PREFS_NAMESPACE, false))
		{
			Outgoing::printOutputStringFlash(F("ERR: Cannot open prefs"));
			Outgoing::printLine();
		}
	}

	void end()
	{
		prefs.end();
	}
#elif defined(EEPROM_FALLBACK)
	static EepromConfig config{};

	void SetDefaults(EepromConfig& configToSet)
	{
		memset(&configToSet, 0, sizeof(configToSet));
		configToSet.configVersion = EEPROM_CONFIG_VERSION;
		configToSet.commandSource = false;
		configToSet.debugEnabled = false;
		configToSet.stopIsShutdown = false;
	}

	bool LoadFromEeprom(EepromConfig& out)
	{
		EEPROM.get(0, out);

		if (out.configVersion != EEPROM_CONFIG_VERSION)
		{
			return false;
		}

		return true;
	}

	void SaveToEeprom(const EepromConfig& in)
	{
		EEPROM.put(0, in);
	}

	void begin()
	{
		if (config.configVersion != EEPROM_CONFIG_VERSION)
		{
			if (!LoadFromEeprom(config))
			{
				SetDefaults(config);
				SaveToEeprom(config);
			}
		}
	}

	void end()
	{
		// No op
	}

#endif

#ifdef UTILITY_PIN

	unsigned long utiityPinStartTime;
	bool utilityPinInit = false;

	byte utilityPressCount = 0;
	int utilityPrefsPressLastState = HIGH;
	unsigned long utilityLastChangeMs = 0;
	unsigned long utilityLastPressMs = 0;

	void update()
	{
		if (!utilityPinInit)
		{
			pinMode(UTILITY_PIN, UTILITY_PIN_INPUT);
			utilityPinInit = true;
		}

		int reading = digitalRead(UTILITY_PIN);
		unsigned long now = millis();

		// debounce
		if (reading != utilityPrefsPressLastState && (now - utilityLastChangeMs) >= UTILITY_PRESS_DEBOUNCE_TIME)
		{
			utilityLastChangeMs = now;
			utilityPrefsPressLastState = reading;

			// count only on "pressed" edge
			if (reading == UTILITY_PRESS_VALUE)
			{
				// if first press or within allowed gap, count it; else start over
				if (utilityPressCount == 0 || (now - utilityLastPressMs) <= UTILITY_PRESS_PRESS_TIMEOUT)
				{
					utilityPressCount++;
				}
				else
				{
					utilityPressCount = 1; // current press becomes the first in a new sequence
				}

				utilityLastPressMs = now;
			}
		}

		// if too much time passes without another press, check the count and clear the sequence
		if (utilityPressCount > 0 && (now - utilityLastPressMs) > UTILITY_PRESS_PRESS_TIMEOUT)
		{
#ifdef TOGGLE_DEBUG
			// if three presses exactly, toggle extra debug logging
			if (utilityPressCount == TOGGLE_DEBUG_PRESS_COUNT)
			{
				toggleDebugEnabled();
				utilityPressCount = 0;
				return;
			}
#endif

#ifdef RESET_PREFS_SUPPORTED
			if (utilityPressCount == RESET_PREFS_MULTI_PRESS_COUNT)
			{
				utiityPinStartTime = 0;
				begin();

#ifdef ESP32
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
				prefs.remove(ESP32_PREFS_KEY_CMD_SOURCE);
#endif

#ifdef RELAY_SUPPORTED
				prefs.remove(ESP32_PREFS_KEY_RELAY_STATE);
				prefs.remove(ESP32_PREFS_KEY_RELAY_BRIDGE_MAC);
				prefs.remove(ESP32_PREFS_KEY_RELAY_THIS_MAC);
#endif

#ifdef REPORT_UID
#ifndef PERSIST_UID_ON_PREFS_RESET
				prefs.remove(ESP32_PREFS_KEY_UID);
#endif
#endif

#ifdef TOGGLE_DEBUG
				prefs.remove(ESP32_PREFS_KEY_TOGGLE_DEBUG);
#endif
#elif defined(EEPROM_FALLBACK)
				SetDefaults(config);
				SaveToEeprom(config);
#endif
				end();
#if defined(ENABLE_STATUS_LIGHTS)
				for (int i = 0; i < 3; i++)
				{
					StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, CRGB::White * 0.33f);
					StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::White * 0.33f);
					StatusLights::setDesiredColor(USER_STATUS_LIGHT, CRGB::White * 0.33f);
					StatusLights::updateLights();
					delay(100);
					StatusLights::setDesiredColor(CONNECTION_STATUS_LIGHT, CRGB::Black);
					StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, CRGB::Black);
					StatusLights::setDesiredColor(USER_STATUS_LIGHT, CRGB::Black);
					StatusLights::updateLights();
					delay(100);
				}

#endif
				utilityPressCount = 0;
				BasicCommands::reboot(false);
				return;
			}
#endif
			utilityPressCount = 0;
		}
	}
#endif

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
	bool getUseExportedCommandStream()
	{
		begin();
		bool result = false;
#ifdef ESP32
		result = prefs.getBool(ESP32_PREFS_KEY_CMD_SOURCE, false);
#elif defined(EEPROM_FALLBACK)
		result = config.commandSource;
#else
#error "No persistent storage backend selected"
#endif
		end();
		return result;
	}

	void setUseExportedCommandStream(bool value)
	{
		begin();
#ifdef ESP32
		prefs.putBool(ESP32_PREFS_KEY_CMD_SOURCE, value);
#elif defined(EEPROM_FALLBACK)
		if (config.commandSource != value)
		{
			config.commandSource = value; // false = Live USB, true = Exported
			SaveToEeprom(config);
		}
#else
#error "No persistent storage backend selected"
#endif
		end();
	}
#endif

#ifdef RELAY_SUPPORTED
	int getRelayState()
	{
#ifdef ESP32
		begin();
		int result = prefs.getInt(ESP32_PREFS_KEY_RELAY_STATE, 0);
		if (result > VALUE_RELAY_STATE_PEER)
		{
			result = 0;
		}
		end();
		return result;
#endif
	}

	void setRelayState(int value)
	{
#ifdef ESP32
		begin();
		if (value > VALUE_RELAY_STATE_PEER)
		{
			value = 0;
		}
		prefs.putInt(ESP32_PREFS_KEY_RELAY_STATE, value);
		end();
#endif
	}

	void storeBridgeMacAddress(uint8_t mac[6])
	{
		begin();
		prefs.putBytes(ESP32_PREFS_KEY_RELAY_BRIDGE_MAC, mac, 6);
		end();
	}

	bool getBridgeMacAddress(uint8_t mac[6])
	{
		begin();
		size_t bytesRead = 0;
		if (prefs.isKey(ESP32_PREFS_KEY_RELAY_BRIDGE_MAC))
		{
			bytesRead = prefs.getBytes(ESP32_PREFS_KEY_RELAY_BRIDGE_MAC, mac, 6);
		}

		end();

		// Verify that we read exactly 6 bytes; if not, clear the array.
		if (bytesRead != 6)
		{
			for (uint8_t i = 0; i < 6; i++)
			{
				mac[i] = 0;
			}
			return false;
		}
		return true;
	}

	void getThisDeviceMacAddress(uint8_t mac[6])
	{
		begin();

		if (prefs.isKey(ESP32_PREFS_KEY_RELAY_THIS_MAC))
		{
			prefs.getBytes(ESP32_PREFS_KEY_RELAY_THIS_MAC, mac, 6);
		}
		else
		{
			UDIDHelper::getThisDeviceMacAddress(mac);
			prefs.putBytes(ESP32_PREFS_KEY_RELAY_THIS_MAC, mac, 6);
		}

		end();
	}

	void getThisDeviceMacAddress(char* str)
	{
		uint8_t mac[6];
		getThisDeviceMacAddress(mac);
		UDIDHelper::convertMACToCStr(mac, str);
	}

#endif

#ifdef REPORT_UID
	void getUID(uint8_t uid[UID_LENGTH])
	{
		begin();

#ifdef ESP32
		size_t len = prefs.getBytes(ESP32_PREFS_KEY_UID, uid, UID_LENGTH);
		if (len != UID_LENGTH)
		{
			esp_fill_random(uid, UID_LENGTH);
			prefs.putBytes(ESP32_PREFS_KEY_UID, uid, UID_LENGTH);
		}
#endif

		end();
	}

#endif

#ifdef NAMED_BOARD
	void getNamedHWVersion(char str[NAMED_BOARD_HW_VER_LENGTH])
	{
#ifdef ESP32
		begin();
		size_t len = prefs.getBytes(ESP32_PREFS_KEY_NAMED_HW_VERSION, str, NAMED_BOARD_HW_VER_LENGTH);
		end();

		if (len == 0)
		{
			// no version stored
			str[0] = '\0';

			// burn in the version based on release assumptions before this feature existed
#ifdef BOTTANGO_SOLAR
			strcpy(str, "1.3");
			setNamedHWVersion(str);
#elif defined(BOTTANGO_NOVA) || defined(BOTTANGO_IMPULSE)
			strcpy(str, "1.2");
			setNamedHWVersion(str);
#endif
		}
		else if (len < NAMED_BOARD_HW_VER_LENGTH)
		{
			// proper length: terminate right after data
			str[len] = '\0';
		}
		else
		{
			// buffer full: force-terminate at last slot
			str[NAMED_BOARD_HW_VER_LENGTH - 1] = '\0';
		}
#endif
	}

	void setNamedHWVersion(const char* version)
	{
#ifdef ESP32
		begin();
		// clamp to max 10 chars
		size_t len = strnlen(version, NAMED_BOARD_HW_VER_LENGTH - 1);
		prefs.putBytes(ESP32_PREFS_KEY_NAMED_HW_VERSION, version, len);
		end();
#endif
	}
#endif

#ifdef TOGGLE_DEBUG

	bool cachedDebugState = false;
	bool initCheckComplete = false;

	bool debugEnabled()
	{
		if (!initCheckComplete)
		{
			cachedDebugState = getDebugEnabledFromPersistent();
			initCheckComplete = true;
		}
		return cachedDebugState;
	}

	bool getDebugEnabledFromPersistent()
	{
		begin();
		bool result = false;
#ifdef ESP32
		result = prefs.getBool(ESP32_PREFS_KEY_TOGGLE_DEBUG, false);
#elif defined(EEPROM_FALLBACK)
		result = config.debugEnabled;
#else
#error "No persistent storage backend selected"
#endif
		end();

		return result;
	}

	void toggleDebugEnabled()
	{
#ifdef RELAY_SUPPORTED
		Outgoing::toggleOnSecondaryOutgoing();
#endif
		begin();
		bool newVal = !(debugEnabled());
#ifdef ESP32
		prefs.putBool(ESP32_PREFS_KEY_TOGGLE_DEBUG, newVal);
#elif defined(EEPROM_FALLBACK)
		if (config.debugEnabled != newVal)
		{
			config.debugEnabled = newVal;
			SaveToEeprom(config);
		}
#else
#error "No persistent storage backend selected"
#endif

		end();

		Outgoing::printOutputStringFlash(F("Set Debug State To: "));
		Outgoing::printOutputStringMem(newVal);
		Outgoing::printLine();

#ifdef RELAY_SUPPORTED
		Outgoing::endToggleOnSecondaryOutgoing();
#endif

		cachedDebugState = !cachedDebugState;
	}
#endif

#ifdef DYNAMIC_STOP_BUTTON_BEHAVIOR
	bool stopIsShutdown()
	{

		begin();
		bool result = false;
#ifdef ESP32
		result = prefs.getBool(ESP32_PREFS_KEY_STOP_BTN, false);
#elif defined(EEPROM_FALLBACK)
		result = config.stopIsShutdown;
#else
#error "No persistent storage backend selected"
#endif
		end();
		return result;
	}

	void setStopIsShutdown(bool stopIsShutdown)
	{

		begin();
#ifdef ESP32
		prefs.putBool(ESP32_PREFS_KEY_STOP_BTN, stopIsShutdown);
#elif defined(EEPROM_FALLBACK)
		if (config.stopIsShutdown != stopIsShutdown)
		{
			config.stopIsShutdown = stopIsShutdown;
			SaveToEeprom(config);
		}
#else
#error "No persistent storage backend selected"
#endif
		end();
	}
#endif

} // PersistentConfigUtil
