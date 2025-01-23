#ifndef Modules_h
#define Modules_h
#include "src/BoardDefs.h"
#ifndef OVERRIDE_MODULES

// !! Communication type !!
// select only 1
#define USE_USB_SERIAL                                  // DEFAULT Use USB serial connection. Comment this line out if using ESP32 Wifi instead
//#define USE_ESP32_WIFI                               // uncomment this line to use WiFi Socket communication on an ESP32

// #define ALLOW_SYNC_COMMANDS                             // parse commands that contain multiple commands wrapped in a single message

// ---------------------------------- //

// !! Live control or exported animations !!
// selecting nothing is USB control
// otherwise select one of the below:

// #define USE_CODE_COMMAND_STREAM                      // uncomment this line to drive animations from exported code instead of live control
// #define USE_SD_CARD_COMMAND_STREAM                   // uncomment this line to drive animations from files on an SD card instead of live control (Note, arduino Uno / Nano not currently supported)


// if you have selected one of the above, enable this to dynamically switch between USB or the selected above exported animation source
// #define ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH       // uncomment this line to monitor a pin at boot to switch between live control or saved animation playback.


// ---------------------------------- //

// !!LIBRARIES!!
// #define USE_ADAFRUIT_PWM_LIBRARY                      //uncomment this line to enable the Adafruit PCA9685 Servo Driver (i2s and pin)

// ---------------------------------- //

// !!AUDIO!!
// (select only 1)
#define AUDIO_TRIGGER_EVENT                        // uncomment this line to have audio keyframes act as trigger events
// #define AUDIO_SD_I2S                               // uncomment this line to have audio keyframes play audio over i2s, reading audio file from SD card

// extra features
// #define DYNAMIC_VOLUME                              // uncomment this line to adjust volume of i2s signal based on analog pin read
// #define PIN_ON_AUDIO_PLAY                              // uncomment this line to take a pin high when playing audio (to enable an amp, etc.)

// ---------------------------------- //

// !! RELAY !!

// (select only 1, or none for default behavior)
// #define RELAY_PARENT                                // uncomment this line to act as a relay parent, that will pass commands to child controllers
// #define RELAY_CHILD                                 // uncomment this line to act as a relay child, that will recieve commands to from a parent controller

// relay Comms type
// (select only 1 if relay parent or child)
// #define RELAY_COMS_ESPNOW                       // select the kind of communication stack for relay parent <-> child. ESP-Now only supported currently
                                                // children will use this and override choice of USB Serial or Wifi and only talk to parent via this method
                                                // parents will still use usb serial or wifi to talk to desktop app, and this setting for child comms.

// ---------------------------------- //

// !! STATUS LIGHTS !!
// #define ENABLE_STATUS_LIGHTS

// ---------------------------------- //

// !! OTA Firmware Update and Bottango Boards !!
// #define ENABLE_ESP_OTA_UPDATE // enable OTA firmware updates
// #define PIN_REMAPPING // remap input pin number to onboard pins
// #define PIN_LOW_LAUNCH // take pins low on launch until registered

#endif
#endif
