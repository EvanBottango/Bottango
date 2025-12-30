#include "../BottangoArduinoModules.h"

#ifndef PERSISTENT_CONFIG_UTIL_H
#define PERSISTENT_CONFIG_UTIL_H

#include <Arduino.h>
#include "../BottangoArduinoConfig.h"

#ifdef ESP32
#include <Preferences.h>
#else

// only TEENSYDUINO and ESP32 tested so far...
// the rest are more in theory than practice...
#if defined(TEENSYDUINO) || defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32)
#include <EEPROM.h>
#define EEPROM_FALLBACK
#endif

#endif

namespace PersistentConfigUtil
{

#ifdef REPORT_UID
#define UID_LENGTH 8
#define UID_HEX_LEN (UID_LENGTH * 2)
#define UID_CSTR_SIZE (UID_HEX_LEN + 1)
#endif

#ifdef ESP32
#define ESP32_PREFS_NAMESPACE "BTNGO"

#ifdef REPORT_UID
#define ESP32_PREFS_KEY_USER_ID "USR_ID"
#define ESP32_PREFS_KEY_UID "UID"
#endif

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#define ESP32_PREFS_KEY_CMD_SOURCE "CMD_SRC"
#endif

#ifdef RELAY_SUPPORTED
#define ESP32_PREFS_KEY_RELAY_STATE "RELAY"
#define ESP32_PREFS_KEY_RELAY_BRIDGE_MAC "BRDGMAC"
#define ESP32_PREFS_KEY_RELAY_THIS_MAC "MAC"
#endif

#ifdef NAMED_BOARD
#define ESP32_PREFS_KEY_NAMED_HW_VERSION "HW_VER"
#endif

#ifdef TOGGLE_DEBUG
#define ESP32_PREFS_KEY_TOGGLE_DEBUG "DEBUG"
#endif

#ifdef DYNAMIC_STOP_BUTTON_BEHAVIOR
#define ESP32_PREFS_KEY_STOP_BTN "STP_BTN"
#endif

    void begin();
    void end();
    extern Preferences prefs;
#elif defined(EEPROM_FALLBACK)

    // Version to bump when the eeprom layout changes
    constexpr uint8_t EEPROM_CONFIG_VERSION = 1;

    struct EepromConfig
    {
        uint8_t configVersion;
        uint8_t uid[8];
        bool commandSource; // commandSource: false = Live USB, true = Exported
        bool debugEnabled;
        bool stopIsShutdown;
    };

    static_assert(sizeof(EepromConfig) == 12, "EepromConfig size changed—update version/migrations.");

    void SetDefaults(EepromConfig &configToSet);
    bool LoadFromEeprom(EepromConfig &out);
    void SaveToEeprom(const EepromConfig &in);
    void begin();
    void end();
#endif

#ifdef RESET_PREFS_SUPPORTED
    void update();
#endif

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
    bool getUseExportedCommandStream();
    void setUseExportedCommandStream(bool value);
#endif

#ifdef RELAY_SUPPORTED
#define VALUE_RELAY_STATE_LIVE_USB 0
#define VALUE_RELAY_STATE_BRIDGE 1
#define VALUE_RELAY_STATE_PEER 2

    int getRelayState();
    void setRelayState(int value);

#ifdef RELAY_COMS_ESPNOW
    void storeBridgeMacAddress(uint8_t mac[6]);
    bool getBridgeMacAddress(uint8_t mac[6]);
    void getThisDeviceMacAddress(uint8_t mac[6]);
    void getThisDeviceMacAddress(char *str);
#endif

#endif

#ifdef REPORT_UID
    void getUID(uint8_t uid[UID_LENGTH]);
    void convertUidToCStr(const uint8_t uid[UID_LENGTH], char str[UID_CSTR_SIZE]);
#endif

#ifdef NAMED_BOARD
    void getNamedHWVersion(char str[NAMED_BOARD_HW_VER_LENGTH]);
    void setNamedHWVersion(const char *version);
#endif

#ifdef TOGGLE_DEBUG
    bool debugEnabled();
    bool getDebugEnabledFromPersistent();
    void toggleDebugEnabled();
#endif

#ifdef DYNAMIC_STOP_BUTTON_BEHAVIOR
    bool stopIsShutdown();
    void setStopIsShutdown(bool stopIsShutdown);
#endif
}

#endif