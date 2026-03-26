#ifndef Modules_h
#define Modules_h
#include "src/BoardDefs.h"
#ifndef OVERRIDE_MODULES

// !! Communication type !!
// select only 1
#define USE_USB_SERIAL                                      // DEFAULT Use USB serial connection. Comment this line out if using ESP32 Wifi instead 
//#define USE_ESP32_WIFI                                    // uncomment this line to use WiFi Socket communication on an ESP32 (this is not the realy (wireless) module.

// ---------------------------------- //

// !! RELAY !!

#define RELAY_SUPPORTED                                  // uncomment this line to allow for relay bridge / relay peer behavior

// relay Comms type.                                        // (select only 1 if relay behavior is supported)
#define RELAY_COMS_ESPNOW                                // select the kind of communication stack for relay parent <-> child. ESP-Now only supported currently

// children will use this and override choice of USB Serial or Wifi and only talk to parent via this relay method
// parents will still use usb serial or wifi to talk to desktop app, and this setting for child comms.

#define ALLOW_SYNC_COMMANDS                             // parse commands that contain multiple commands wrapped in a single message

// ---------------------------------- //

// !! Live control or exported animations !!
// selecting nothing is USB control
// otherwise select one of the below:

// #define USE_CODE_COMMAND_STREAM                             // uncomment this line to drive animations from exported code instead of live control
#define USE_SD_CARD_COMMAND_STREAM                       // uncomment this line to drive animations from files on an SD card instead of live control (Note, arduino Uno / Nano / mega not supported)


// if you have selected one of the above, enable this to dynamically switch between USB or the selected above exported animation source
#define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH              // uncomment this line to monitor a pin at boot to switch between live control or saved animation playback.


// ---------------------------------- //

// !! PCA9685 !!
// #define USE_ADAFRUIT_PWM_LIBRARY                         //uncomment this line to enable the Adafruit PCA9685 Servo Driver (i2s and pin)

// ---------------------------------- //

// !! AUDIO !!
// (select only 1)
// #define AUDIO_TRIGGER_EVENT                                 // uncomment this line to have audio keyframes act as trigger events
// #define AUDIO_SD_I2S                                     // uncomment this line to have audio keyframes play audio over i2s, reading audio file from SD card

// extra features
// #define DYNAMIC_VOLUME                                   // uncomment this line to adjust volume of i2s signal based on analog pin read
// #define PIN_ON_AUDIO_PLAY                                // uncomment this line to take a pin high when playing audio (to enable an amp, etc.)

// ---------------------------------- //

// !! STATUS LIGHTS !!
#define ENABLE_STATUS_LIGHTS                             // uncomment this line to drive status lights based on driver status

// ---------------------------------- //

// !! STOP BUTTON !!
#define STOP_BUTTON_SUPPORTED                            // uncomment this line to monitor a button or pin for stop action
// #define DYNAMIC_STOP_BUTTON_BEHAVIOR                     // uncomment this line to allow for dynamic change to what the stop button does

// ---------------------------------- //

// !! PIN CHANGES !!
// #define PIN_REMAPPING                                    // remap input user friendly pin number to onboard pins
// #define PIN_LOW_LAUNCH                                   // take pins low on launch until registered

// ---------------------------------- //

// !! Unique ID !!
// #define REPORT_UID                                       // report a unique ID to help identify the board without relying on com port

// ---------------------------------- //

// !! Utility button !!
// #define UTILITY_PIN 0                                       // monitor a specific button/pin for some utility behaviors

// #define RESET_PREFS_SUPPORTED                               // support reset persistent storage with utility button
#define TOGGLE_DEBUG                                        // support toggle verbose logging

#endif
#endif
