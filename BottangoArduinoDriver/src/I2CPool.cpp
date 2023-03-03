#include "I2CPool.h"

// PWM DRIVERS //

#ifdef USE_ADAFRUIT_PWM_LIBRARY
CircularArray<Adafruit_PwmServoDriverContainer> pwmDriverContainers = CircularArray<Adafruit_PwmServoDriverContainer>(MAX_I2C_DRIVERS);
#endif

Adafruit_PwmServoDriverContainer *getPWMDriverContainer(byte i2cAddress)
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    for (byte i = 0; i < pwmDriverContainers.size(); i++)
    {
        if (pwmDriverContainers.get(i)->i2cAddress == i2cAddress)
        {
            return pwmDriverContainers.get(i);
        }
    }
#endif
    return NULL;
}

void registerPWMDriverEffector(byte i2cAddress)
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY

    Adafruit_PwmServoDriverContainer *driver = getPWMDriverContainer(i2cAddress);

    if (driver == NULL)
    {
        if (pwmDriverContainers.size() >= MAX_I2C_DRIVERS)
        {
            Error::reportError_TooManyI2c();
            return;
        }
        driver = new Adafruit_PwmServoDriverContainer(i2cAddress);
        pwmDriverContainers.pushBack(driver);
    }

    driver->registeredCount++;
#endif
}

void removePWMDriverEffector(byte i2cAddress)
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    Adafruit_PwmServoDriverContainer *driver = getPWMDriverContainer(i2cAddress);

    if (driver == NULL)
    {
        Error::reportError_NoServoOnPin();
        return;
    }

    driver->registeredCount--;

    if (driver->registeredCount <= 0)
    {
        pwmDriverContainers.remove(driver);
        delete driver;
    }
#endif
}