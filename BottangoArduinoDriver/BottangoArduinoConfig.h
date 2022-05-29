#ifndef DevConsts_h
#define DevConsts_h

#define MAX_REGISTERED_EFFECTORS 8 // Max effectors that can be registered at once. See readme.txt before increasing.
#define MAX_COMMAND_LENGTH 100     // max character count of a command
#define READ_TIMEOUT 50            // time in MS before timeout while reading incoming serial
#define COMMANDS_BUFFER_SIZE 30    // max number of commands to check incoming command names against
#define COMMANDS_PARAMS_SIZE 15    // max number of paramaters in an incoming command
#define BAUD_RATE 115200           // serial speed at which bot tango and arduino communicate with each other

// #define USE_COMMAND_STREAM                   //uncomment this line to if you have added exported animations
// #define USE_ADAFRUIT_PWM_LIBRARY             //uncomment this line to use Adafruit pwm library
// #define USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY //uncomment this line to use Adafruit motor shield v2 library

// #define ENABLE_STEPPERS // uncomment this line to use Stepper motors. Enabling stepper motors is supported on a limited range of Arduino compatible boards (Uno, Mega and Nano). As well, you probably shouldn't use servos and steppers on the same board.

#endif