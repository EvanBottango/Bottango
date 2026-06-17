#ifndef Adafruit_PwmServoDriverContainer_h
#define Adafruit_PwmServoDriverContainer_h

#include "../BottangoArduinoConfig.h"
#include "Arduino.h"

#ifdef USE_ADAFRUIT_PWM_LIBRARY
#include <Adafruit_PWMServoDriver.h>
#endif

class Adafruit_PwmServoDriverContainer
{
public:
    Adafruit_PwmServoDriverContainer(byte i2cAddress);
    ~Adafruit_PwmServoDriverContainer();

    byte i2cAddress = 0;
    byte registeredCount = 0;

#ifdef USE_ADAFRUIT_PWM_LIBRARY
    Adafruit_PWMServoDriver *driver;
#endif
};

#endif