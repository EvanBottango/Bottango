#ifndef DevConsts_h
#define DevConsts_h

#include "BottangoArduinoModules.h"
#include <Arduino.h>

// !! Max Registered Motors !!
// Max effectors (motors, etc.) that can be registered at once.
// more than 8 crashes an Arduino Uno / Nano
// otherwise capped by default at 16, but can be increased if needed (depending on your hardware)

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#ifdef USE_ADAFRUIT_PWM_LIBRARY
#define MAX_REGISTERED_EFFECTORS 6 // Arduino Uno or Nano + PCA driver can only support 6
#else
#define MAX_REGISTERED_EFFECTORS 8 // Arduino Uno or Nano bare pin pwm supports 8
#endif
#else
#define MAX_REGISTERED_EFFECTORS 16 // most modern boards can handle 16 or more (EX, ESP32, etc.)
#endif

// ---------------------------------- //

// !! Wifi Configuration !!
#ifdef USE_ESP32_WIFI
#define WIFI_SSID "MY_NETWORK_ID"                   // wifi ssid
#define WIFI_PASSWORD "MY_NETWORK_PW"               // wifi password
#define WIFI_SERVER_IP "XXX.XXX.XXX.X"              // wifi server ip on the local network, likely something like 192.168.1.X
#define WIFI_SERVER_PORT 59225                      // wifi server port on the local network
#endif

// ---------------------------------- //

// !! SD Card Pin Configuration !!
// Note that Arduino Uno / Nano are not supported with SD Card
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)
#ifdef ESP32                                        // spi pins
#define SDPIN_CS 5                                  // chip select pin
#define SDPIN_MOSI 23                               // MOSI pin
#define SDPIN_CLK 18                                // clock pin
#define SDPIN_MISO 19                               // MISO pin
#elif defined(TEENSYDUINO)
#define SDPIN_CS BUILTIN_SDCARD
#else // anything else
#define SDPIN_CS 5                                  // chip select pin
#define SDPIN_MOSI 23                               // MOSI pin
#define SDPIN_CLK 18                                // clock pin
#define SDPIN_MISO 19                               // MISO pin
#endif
#endif

// ---------------------------------- //

// !! Dynamic switch between live and exported animations !!
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#define ANIMATION_STATE_SELECTION_PIN 36            // Pin to monitor on boot to determine if should be in live control or exported
#define SELECT_EXPORTED_IS_LOW                     // LOW on pin selects exported. Comment out this line to choose exported on highs.
#endif

// ---------------------------------- //

// !! I2S and Audio Pins !!
#ifdef AUDIO_SD_I2S
#define I2S_BCLK 25                                 // bit clock line
#define I2S_LRC 17                                  // left right clock
#define I2S_DOUT 16                                 // data
#define I2S_BUFFER_SIZE 2048
#define I2S_WRITE_TIMEOUT (25 / portTICK_PERIOD_MS) // Short timeout for non-blocking operation
#define I2S_DMA_BUFF_COUNT 6;
#define I2S_DMA_BUFF_LEN 240;
#define I2S_INIT_SAMPLE_RATE 48000;
#ifdef DYNAMIC_VOLUME
#define VOLUME_PIN 35
#define VOLUME_MIN 1.0;
#define VOLUME_MAX 0.01;
#define VOLUME_READ_INTERVAL 50
#endif
#ifdef PIN_ON_AUDIO_PLAY
#define AUDIO_ENABLE_PIN 2
#endif
#endif

// ---------------------------------- //

// Curve Evaluation:
// enable one of the bezier curve default evaluation methods:
#define DEFAULT_FLOAT_CURVE // float math option (default, battle hardened and highly tested)
// #define DEFAULT_FIXED_CURVE                      // fixed math option. Slightly lower curve precision, but faster.  Likely will see improved HW communication and higher max effector count. New and being tested.


#define AVR_STEPPER_FIXED                           // in order to keep up with the demands of stepper motors, AVR boards (Arduino Uno, Nano, Mega, etc) use
                                                    // fixed evaluation overriding the above default. Comment this line out to use the default curve evaluation on avr steppers

#define VELOCITY_SEGMENT_MS 67                      // Velocity effectors (IE steppers) will segment curves into chunks of velocity. This field defines how long each chunk is in MS.
                                                    // lower number means more segments. More segments will be more accurate to the curve, but slower to process and less smooth stepper movement
                                                    // If you wanted to target 15 segments per second (the default) divide 1000 by 15: 1000 / 15 = 66.666, so rounded to 67
                                                    // 30 segments per second would be 33, etc.

#define STEPPER_SYNC_SPEED 2                        // steppers will sync at 1/X percent of max speed. IE, 2 == 50% max speed, 4 == 25% max Speed, etc.

// ---------------------------------- //

// recieved Command parsing:
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#define MAX_COMMAND_LENGTH 100                               // max character count of a command
#else
#define MAX_COMMAND_LENGTH 248                               // max character count of a command
#endif
#define CMD_PREFIX_SIZE 16                                   // max character count of a command prefix
#define READ_TIMEOUT 50                                      // time in MS before timeout while reading incoming serial
#define COMMANDS_PARAMS_SIZE 15                              // max number of paramaters in an incoming command
#define BAUD_RATE 115200                                     // serial speed at which Bottango and Arduino communicate with each other
#define COMPRESSED_SIGNAL_MAX 8192.0f                        // Movement is sent as a value between 0 and this value
#define COMPRESSED_SIGNAL_MAX_INT (int)COMPRESSED_SIGNAL_MAX // Cast to int shortcut
#define TRIGGER_EVENT_PIN_TIME 250                            // time in MS to keep a pin high before setting it back low in trigger events
#define OUTGOING_TIMEOUT_RESPONSE_TIME 1000                  // time in MS to wait to get a continue response during a multi message outgoing response
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#define MAX_NUM_CURVES 3                                     // number of curves to precache (uno / mega)
#else
#define MAX_NUM_CURVES 8                                     // number of curves to precache (everything else)
#endif

// ---------------------------------- //

// sd card configuration
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)
#define MAX_FILE_PATH_SIZE 30                               // max character count of a file path
const char SD_ANIMATION_PATH[] PROGMEM = "/anim/";          // file path to look for animation directories on sd card.
const char SD_AUDIO_PATH[] PROGMEM = "/audio/";             // file path to look for audio files
const char SD_SETUP_PATH[] PROGMEM = "setup/";              // file path to look for initial set up data
const char SD_DATA_ANIMDATA[] PROGMEM = "data.txt";         // file name of animation data
const char SD_DATA_LOOPDATA[] PROGMEM = "loop.txt";         // file name of loop data
const char SD_HASH_FORMAT[] PROGMEM = "hash.txt";           // file format name of hash file name
#define FILE_HASH_LENGTH 32                                 // character count of a file hash code
const char SD_DATA_CONFIGDATA[] PROGMEM = "config.txt";     // file name of config data
const char SD_AUDIO_FORMAT[] PROGMEM = ".wav";              // file format name of audio file
#define SD_CARD_REMOUNT_TIME 2000                           // time to wait in MS before next attempt to remount
#define SD_ANIM_PREREAD_MS 25                               // ms early to cache execute curves from sd card
#define SD_ANIM_PREREAD_MS_RELAY 250                        // ms early to cache execute curves from sd card when relay bridge
#define TXT_BUFFER_SIZE_SD 1024                             // max num chars to store in the sd card read buffer
#define TXT_BUFFER_READ_CHUNK_SIZE 128                      // max num chars to attempt to read off sd card at once
#endif

// ---------------------------------- //

// animation export config
#define MAX_EXPORTED_ANIMATIONS 32                          // max number of animations that can be exported and played back without PC connection
#define EXPORTED_ANIMATION_INPUT INPUT                      // what kind of input to use for pin monitoring for triggering animations. Change to INPUT_PULLDOWN or INPUT_PULLUP if desired.

// ---------------------------------- //

// stop button config
#ifdef STOP_BUTTON_SUPPORTED

// stop pin
#define STOP_BUTTON_PIN 39
#define BUTTON_DEBOUNCE_TIME 500
#define STOP_INPUT_TYPE INPUT                               // what kind of input to use for pin monitoring for stop. Change to INPUT_PULLDOWN or INPUT_PULLUP if desired.

#ifndef DYNAMIC_STOP_BUTTON_BEHAVIOR                        // not settable in app?
#define STOP_BUTTON_SHOULD_DISCONNECT false                 // true == shut down fully when stop is pressed while offline, false == just stop current animation 
#endif

// stop input type
#ifdef BOTTANGO_SOLAR                                       // solar is analog
#define STOP_READ_TYPE_ANALOG
#define STOP_READ_ACTIVE_MIN 0                              // min analog read to fire
#define STOP_READ_ACTIVE_MAX 250                            // max analog read to fire
#else
#define STOP_READ_TYPE_DIGITAL                              // nova/impulse/custom is digital
#define STOP_READ_ACTIVE LOW                                // fire on high or low?
#endif

#endif
// ---------------------------------- //

// Utility Pin
#if defined(UTILITY_PIN)

#define UTILITY_PIN_INPUT INPUT_PULLUP
#define UTILITY_PRESS_VALUE LOW
#define UTILITY_PRESS_DEBOUNCE_TIME 30
#define UTILITY_PRESS_PRESS_TIMEOUT 500
#endif

#if defined(RESET_PREFS_SUPPORTED)
#define PERSIST_UID_ON_PREFS_RESET
#define RESET_PREFS_MULTI_PRESS_COUNT 5
#endif

#if defined(TOGGLE_DEBUG)
#define ALWAYS_LOG_ERROR_CASE true
#define TOGGLE_DEBUG_PRESS_COUNT 3
#define EXPORTED_ANIM_LOGGING
#define RELAY_LOGGING
#define ESP32WIFI_LOGGING
#endif

// ---------------------------------- //

// relay parent and child config
#ifdef RELAY_SUPPORTED

#define MAX_RELAY_CHILD 16                                  // max number of controllers to relay commands to
#define RELAY_BOOT_INTERVAL_AS_BRIDGE 500UL                 // how often bridge sends a boot request to check if a peer has come online
#define RELAY_POLL_INTERVAL_AS_BRIDGE 500UL                // how often bridge enqueues a poll request
#define RELAY_POLL_TIMEOUT_AS_PEER 3000UL                   // how long a peer should wait without a poll before rebooting due to lost bridge

#define TXT_BUFFER_SIZE_RX_FROM_PEER 512                    // num chars to store in per peer (as bridge) in espnow recv buffers in Relay Child class
#define OUT_MESSAGE_QUEUE_DEPTH 6                           // max num full commands to store for transmission to peers via comms (as bridge)

#define TXT_BUFFER_SIZE_RX_COMMS 512                        // num chars to store in the active relay comms, rx
#define TXT_BUFFER_SIZE_TX_COMMS 512                        // num chars to store in the active relay comms, tx

#ifdef RELAY_COMS_ESPNOW
#define ESPNOW_CHANNEL 1                                    // Wifi channel to use. All relays (send and recv) must be on the same channel
#define ESPNOW_RX_QUEUE_DEPTH 4                             // RX packet queue depth for ESP-NOW relay comms
#define ESPNOW_RETRY_MS 50                                  // time in MS before retry sending
#define ESPNOW_TIMEOUT_MS 250                               // time in MS before timeout
#define RELAY_ESPNOW_RESPONSE_TIMEOUT 250UL                 // how long to wait for a response from a peer on ESP-NOW (tune if enabled)
#endif

#ifdef RELAY_COMS_RS485
#define RELAY_RS485_RESPONSE_TIMEOUT 75UL                   // how long to wait for a response from a peer on RS485
#endif

#endif

// ---------------------------------- //

// extra logging
#define EXPORTED_ANIM_LOGGING                               // extra logging of status of exported animation playback
// #define RELAY_LOGGING                                    // extra logging of relay status
#define RELAY_COMMS_LOGGING                              // extra logging of relay comms state machines (RS485, etc)
// #define RELAY_COMMS_LOGGING_DEBUG                       // extra verbose relay comms logging (payloads, state traces)
// #define ESP32WIFI_LOGGING                                   // when communicating without serial, enable it for extra logging

// ---------------------------------- //


// status lights
#ifdef ENABLE_STATUS_LIGHTS
#define STATUS_LED_PIN 4
#define NUM_STATUS_LED 4
#define CONNECTION_STATUS_LIGHT 0
#define SIGNAL_STATUS_LIGHT 1
#define USER_STATUS_LIGHT 2
#define PWR_STATUS_LIGHT 3
#endif

// ---------------------------------- //

// extra Pin Remapping and Low on Launch
#if defined(PIN_REMAPPING) || defined(PIN_LOW_LAUNCH)
#define PIN_REMAP_LENGTH 10
constexpr int inputPins[PIN_REMAP_LENGTH] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
#ifdef BOTTANGO_IMPULSE
constexpr int onboardPins[PIN_REMAP_LENGTH] = {32, 33, 25, 26, 27, 14, 13, 15, 17, 5};
#elif defined(BOTTANGO_SOLAR)
constexpr int onboardPins[PIN_REMAP_LENGTH] = {15, 21, 22, 13, 14, 27, 26, 33, 32, 12};
#endif
#endif

// ---------------------------------- //

// RS485
#if defined(BOTTANGO_NOVA)
#define RS485_RX_PIN 33
#define RS485_DE_PIN 12
#define RS485_TX_PIN 32
#elif defined(BOTTANGO_IMPULSE)
#define RS485_RX_PIN 16
#define RS485_DE_PIN 19
#define RS485_TX_PIN 23
#elif defined(BOTTANGO_SOLAR)
#define RS485_RX_PIN 34
#define RS485_DE_PIN 12
#define RS485_TX_PIN 32
#else
#define RS485_RX_PIN 34
#define RS485_DE_PIN 12
#define RS485_TX_PIN 32
#endif

#define RS485_BAUD 1000000
#define RS485_SERIAL Serial2

#define RS485_SOH '\x01'     // start id header
#define RS485_STX '\x02'     // start message payload
#define RS485_EOT '\x04'     // end of tx
#define RS485_ENQ '\x05'     // start of boot request header
#define RS485_PEERACK '\x06' // start of peer ack response header

// ---------------------------------- //

// named board details

#ifdef NAMED_BOARD
#define NAMED_BOARD_HW_VER_LENGTH 11
#endif

// ---------------------------------- //

// compile time module guards
#if defined(USE_ESP32_WIFI) && !defined(ESP32)
#error "ESP32 Wifi Module only supported on ESP32 architecture"
#endif

#if defined(AUDIO_SD_I2S) && !defined(ESP32)
#error "I2S audio module only supported on ESP32 architecture"
#endif

#if defined(RELAY_SUPPORTED) && !defined(ESP32)
#error "Wireless relay only supported on ESP32 architecture"
#endif

#if defined(NAMED_BOARD) && !defined(ESP32)
#error "Bottango named boards must be esp32 based"
#endif

#if defined(ENABLE_ESP_OTA_UPDATE) && !defined(NAMED_BOARD)
#error "OTA Binaries only provided for named boards"
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) && defined(ARDUINO_ARCH_AVR)
#error "SD card playback module not supported on uno r3 / nano / mega"
#endif

#if defined(ESP32) || defined(TEENSYDUINO) || defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32)
#if defined(REPORT_UID) && !defined(ESP32)
#error "UID assignment module only supported on ESP32 architecture"
#endif
#else
#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(REPORT_UID) || defined(TOGGLE_DEBUG) || defined(DYNAMIC_STOP_BUTTON_BEHAVIOR)
#error "Persistent config modules not supported on this architecture"
#endif
#endif

#endif // config
