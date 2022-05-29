#ifndef Adafruit_MotorShieldV2DriverContainer_h
#define Adafruit_MotorShieldV2DriverContainer_h

#include "../BottangoArduinoConfig.h"
#include "Arduino.h"

#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
#include <Adafruit_MotorShield.h>
#endif

class Adafruit_MotorShieldV2DriverContainer
{

public:
    Adafruit_MotorShieldV2DriverContainer(byte i2cAddress);
    ~Adafruit_MotorShieldV2DriverContainer();
    byte i2cAddress = 0;
    byte registeredCount = 0;

#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
    Adafruit_MotorShield *driver;
#endif
};

#endif