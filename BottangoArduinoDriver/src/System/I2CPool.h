#pragma once

#include "../Util/CircularArray.h"
#include "../Effector/Adafruit_PWMServoDriverContainer.h"

#define MAX_I2C_DRIVERS 3

#ifdef USE_ADAFRUIT_PWM_LIBRARY
extern CircularArray<Adafruit_PwmServoDriverContainer> pwmDriverContainers;
#endif

Adafruit_PwmServoDriverContainer* getPWMDriverContainer(byte i2cAddress);
void registerPWMDriverEffector(byte i2cAddress);
void removePWMDriverEffector(byte i2cAddress);