#ifndef DevConsts_h
#define DevConsts_h

// Basic configurations: //

// !!PLAY SAVED TO CODE ANIMATIONS!!
// #define USE_COMMAND_STREAM                       //uncomment this line to drive animations from exported code instead of USB control


// !!LIBRARIES!!
// #define USE_ADAFRUIT_PWM_LIBRARY                 //uncomment this line to use Adafruit pwm library


// Advanced configurations: //

// !!CURVE EVALUATION!!
// enable one of the bezier curve default evaluation methods:
#define DEFAULT_FLOAT_CURVE                         // float math option (default, battle hardened and highly tested)
// #define DEFAULT_FIXED_CURVE                      // fixed math option. Slightly lower curve precision, but faster.  Likely will see improved HW communication and higher max effector count. New and being tested.

#define AVR_STEPPER_FIXED                           // in order to keep up with the demands of stepper motors, AVR boards (Arduino Uno, Nano, Mega, etc) use
                                                    // fixed evaluation overriding the above default. Comment this line out to use the default curve evaluation on avr steppers

#define VELOCITY_SEGMENT_MS 67                      // Velocity effectors (IE steppers) will segment curves into chunks of velocity. This field defines how long each chunk is in MS.
                                                    // lower number means more segments. More segments will be more accurate to the curve, but slower to process and less smooth stepper movement
                                                    // If you wanted to target 15 segments per second (the default) divide 1000 by 15: 1000 / 15 = 66.666, so rounded to 67
                                                    // 30 segments per second would be 33, etc.

#define STEPPER_SYNC_SPEED 2                        // steppers will sync at 1/X percent of max speed. IE, 2 == 50% max speed, 4 == 25% max Speed, etc.

// !! MAX MOTORS !!
// Max effectors that can be registered at once. See readme.txt before increasing. It's 8 for a good reason.
// switching to fast curve evaluation **may** allow you to increase this (experimental)
#define MAX_REGISTERED_EFFECTORS 8

// Internal configurations: //
// Probably leave these alone
#define MAX_COMMAND_LENGTH 100                      // max character count of a command
#define READ_TIMEOUT 50                             // time in MS before timeout while reading incoming serial
#define COMMANDS_PARAMS_SIZE 15                     // max number of paramaters in an incoming command
#define BAUD_RATE 115200                            // serial speed at which Bottango and Arduino communicate with each other
#define COMPRESSED_SIGNAL_MAX 8192.0f               // Movement is sent as a value between 0 and this value
#define COMPRESSED_SIGNAL_MAX_INT (int)COMPRESSED_SIGNAL_MAX // Cast to int shortcut
#define TRIGGER_EVENT_PIN_TIME 25                   // time in MS to keep a pin high before setting it back low in trigger events

#endif