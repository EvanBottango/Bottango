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

// !! Module Configuration !!

// !! Wifi Configuration !!
#ifdef USE_ESP32_WIFI
#define WIFI_SSID "MY_NETWORK_ID"                   // wifi ssid
#define WIFI_PASSWORD "MY_NETWORK_PW"               // wifi password
#define WIFI_SERVER_IP "XXX.XXX.XXX.X"              // wifi server ip on the local network, likely something like 192.168.1.X
#define WIFI_SERVER_PORT 59225                      // wifi server port on the local network
#endif

// !! Module Pins Configuration !!

// !! SD Card Pin Configuration !!
// Note that Arduino Uno / Nano are not supported with SD Card
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)
#define SDPIN_CS 5                                  // chip select pin
#ifdef ESP32                                        // spi pins
#define SDPIN_MOSI 23                               // MOSI pin
#define SDPIN_CLK 18                                // clock pin
#define SDPIN_MISO 19                               // MISO pin
#endif
#endif

// !! Dynamic switch between live and exported animations !!
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
#define ANIMATION_STATE_SELECTION_PIN 36            // Pin to monitor on boot to determine if should be in live control or exported
#define SELECT_EXPORTED_IS_LOW                     // LOW on pin selects exported. Comment out this line to choose exported on highs.
#endif

// !! I2S and Audio Pins !!
#ifdef AUDIO_SD_I2S
#define I2S_BCLK 25                                 // bit clock line
#define I2S_LRC 17                                  // left right clock
#define I2S_DOUT 16                                 // data
#ifdef DYNAMIC_VOLUME
#define VOLUME_PIN 35
#define VOLUME_MIN 1.0;
#define VOLUME_MAX 0.01;
#endif
#ifdef PIN_ON_AUDIO_PLAY
#define AUDIO_ENABLE_PIN 2
#endif
#endif


// !! SD Card Pin Configuration !!
// Note that Arduino Uno / Nano are not supported with SD Card

// ---------------------------------- //

// !! The below are ADVANCED configurations !!

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


// recieved Command parsing:
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#define MAX_COMMAND_LENGTH 100                               // max character count of a command
#else
#define MAX_COMMAND_LENGTH 248                               // max character count of a command
#endif
#define READ_TIMEOUT 50                                      // time in MS before timeout while reading incoming serial
#define COMMANDS_PARAMS_SIZE 15                              // max number of paramaters in an incoming command
#define BAUD_RATE 115200                                     // serial speed at which Bottango and Arduino communicate with each other
#define COMPRESSED_SIGNAL_MAX 8192.0f                        // Movement is sent as a value between 0 and this value
#define COMPRESSED_SIGNAL_MAX_INT (int)COMPRESSED_SIGNAL_MAX // Cast to int shortcut
#define TRIGGER_EVENT_PIN_TIME 25                            // time in MS to keep a pin high before setting it back low in trigger events
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
#define MAX_NUM_CURVES 3                                     // number of curves to precache (uno / mega)
#else
#define MAX_NUM_CURVES 8                                     // number of curves to precache (everything else)
#endif


// sd card configuration
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)
#define MAX_FILE_PATH_SIZE 30                               // max character count of a file path
const char SD_ANIMATION_PATH[] PROGMEM = "/anim/";          // file path to look for animation directories on sd card.
const char SD_AUDIO_PATH[] PROGMEM = "/audio/";             // file path to look for audio files
const char SD_SETUP_PATH[] PROGMEM = "setup/";              // file path to look for initial set up data
const char SD_DATA_ANIMDATA[] PROGMEM = "data.txt";         // file name of animation data
const char SD_DATA_LOOPDATA[] PROGMEM = "loop.txt";         // file name of loop data
const char SD_DATA_CONFIGDATA[] PROGMEM = "config.txt";     // file name of config data
const char SD_AUDIO_FORMAT[] PROGMEM = ".wav        ";      // file format name of audio file
#define SD_ANIM_PREREAD_MS 25                               // ms early to cache execute curves from sd card
#define TXT_BUFFER_SIZE_SD 1024                              // max num chars to store in the sd card read buffer
#define TXT_BUFFER_READ_CHUNK_SIZE 128                      // max num chars to attempt to read off sd card at once
#endif

// animation export config
#define MAX_EXPORTED_ANIMATIONS 32                          // max number of animations that can be exported and played back without PC connection
#define EXPORTED_ANIMATION_INPUT INPUT                      // what kind of input to use for pin monitoring for triggering animations. Change to INPUT_PULLDOWN or INPUT_PULLUP if desired.

// relay parent and child config
#ifdef RELAY_PARENT
#define MAX_RELAY_CHILD 16                                  // max number of controllers to relay commands to
#endif

#ifdef RELAY_COMS_ESPNOW
#define ESPNOW_CHANNEL 1                                    // Wifi channel to use. All relays (send and recv) must be on the same channel
#define ESPNOW_RETRY_MS 50                                  // time in MS before retry sending
#define ESPNOW_TIMEOUT_MS 250                                  // time in MS before timeout
#define TXT_BUFFER_SIZE_ESPNOW 256                                // max num chars to store in the espnow recv and send buffers

#ifdef RELAY_CHILD
#define ESPNOW_PARENT_ADDRESS {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} // mac address of parent
#endif
#endif

// extra logging
#if defined(RELAY_CHILD) || defined(USE_ESP32_WIFI)
#define ENABLE_SERIAL_LOGGING                               // when communicating without serial, enable it for extra logging
#endif

#define EXPORTED_ANIM_LOGGING                               // extra logging of status of exported animation playback

#ifdef ENABLE_STATUS_LIGHTS
#define STATUS_LED_PIN 4
#define NUM_STATUS_LED 4
#define CONNECTION_STATUS_LIGHT 0
#define SIGNAL_STATUS_LIGHT 1
#define USER_STATUS_LIGHT 2
#define PWR_STATUS_LIGHT 3
#endif

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

#endif // config