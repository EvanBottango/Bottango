#ifndef I2CServoEffector_h
#define I2CServoEffector_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"
#include "../BottangoArduinoConfig.h"

#ifdef USE_ADAFRUIT_PWM_LIBRARY
#include <Adafruit_PWMServoDriver.h>
#endif

class I2CServoEffector : public LoopDrivenEffector
{
public:
    I2CServoEffector(byte i2cAddress, byte pin, short minPWM, short maxPWM, int maxPWMSec, short startPWM);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void destroy() override;
    virtual void dump() override;

protected:
private:
    byte pin = 0;
    byte i2cAddress = 0;
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    Adafruit_PWMServoDriver *driver;
#endif
};

#endif