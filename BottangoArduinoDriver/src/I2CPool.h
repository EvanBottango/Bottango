#ifndef I2cPool_h
#define I2cPool_h

#include "CircularArray.h"
#include "Adafruit_PWMServoDriverContainer.h"
#include "Adafruit_MotorShieldV2DriverContainer.h"

#define MAX_I2C_DRIVERS 3

extern CircularArray<Adafruit_PwmServoDriverContainer> pwmDriverContainers;

Adafruit_PwmServoDriverContainer *getPWMDriverContainer(byte i2cAddress);
void registerPWMDriverEffector(byte i2cAddress);
void removePWMDriverEffector(byte i2cAddress);

extern CircularArray<Adafruit_MotorShieldV2DriverContainer> motorShieldDriverContainers;

Adafruit_MotorShieldV2DriverContainer *getMotorShieldDriverContainer(byte i2cAddress);
void registerMotorShieldDriverEffector(byte i2cAddress);
void removeMotorShieldDriverEffector(byte i2cAddress);

#endif