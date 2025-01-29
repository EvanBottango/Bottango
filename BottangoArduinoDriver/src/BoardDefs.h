// enables quick modules settings for Bottango boards

#ifndef BoardDefs_h
#define BoardDefs_h

// #define BOTTANGO_IMPULSE
// #define BOTTANGO_NOVA
// #define BOTTANGO_SOLAR

#if defined(BOTTANGO_IMPULSE) || defined(BOTTANGO_NOVA) || defined(BOTTANGO_SOLAR)
#define OVERRIDE_MODULES
#endif

#ifdef OVERRIDE_MODULES
#define USE_USB_SERIAL
#define ALLOW_SYNC_COMMANDS
#define ENABLE_STATUS_LIGHTS
#define ENABLE_ESP_OTA_UPDATE
#endif

#ifdef BOTTANGO_SOLAR
#define USE_SD_CARD_COMMAND_STREAM
// #define USE_CODE_COMMAND_STREAM
#define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#define AUDIO_SD_I2S
#define DYNAMIC_VOLUME
#define PIN_ON_AUDIO_PLAY
#define PIN_REMAPPING
#define PIN_LOW_LAUNCH
#endif

#ifdef BOTTANGO_IMPULSE
#define PIN_REMAPPING
#define PIN_LOW_LAUNCH
#endif

#ifdef BOTTANGO_NOVA
#define USE_SD_CARD_COMMAND_STREAM
#define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#endif

#endif