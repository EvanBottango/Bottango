#include "I2CPool.h"

// PWM DRIVERS //

CircularArray<Adafruit_PwmServoDriverContainer> pwmDriverContainers = CircularArray<Adafruit_PwmServoDriverContainer>(MAX_I2C_DRIVERS);

Adafruit_PwmServoDriverContainer *getPWMDriverContainer(byte i2cAddress)
{
    for (byte i = 0; i < pwmDriverContainers.size(); i++)
    {
        if (pwmDriverContainers.get(i)->i2cAddress == i2cAddress)
        {
            return pwmDriverContainers.get(i);
        }
    }
    return NULL;
}

void registerPWMDriverEffector(byte i2cAddress)
{
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
}

void removePWMDriverEffector(byte i2cAddress)
{
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
}

// MOTOR SHIELD DRIVERS //

CircularArray<Adafruit_MotorShieldV2DriverContainer> motorShieldDriverContainers = CircularArray<Adafruit_MotorShieldV2DriverContainer>(MAX_I2C_DRIVERS);

Adafruit_MotorShieldV2DriverContainer *getMotorShieldDriverContainer(uint8_t i2cAddress)
{
    for (byte i = 0; i < motorShieldDriverContainers.size(); i++)
    {
        if (motorShieldDriverContainers.get(i)->i2cAddress == i2cAddress)
        {
            return motorShieldDriverContainers.get(i);
        }
    }
    return NULL;
}

void registerMotorShieldDriverEffector(uint8_t i2cAddress)
{
    Adafruit_MotorShieldV2DriverContainer *driver = getMotorShieldDriverContainer(i2cAddress);

    if (driver == NULL)
    {
        if (motorShieldDriverContainers.size() >= MAX_I2C_DRIVERS)
        {
            Error::reportError_TooManyI2c();
            return;
        }
        driver = new Adafruit_MotorShieldV2DriverContainer(i2cAddress);
        motorShieldDriverContainers.pushBack(driver);
    }

    driver->registeredCount++;
}

void removeMotorShieldDriverEffector(uint8_t i2cAddress)
{
    Adafruit_MotorShieldV2DriverContainer *driver = getMotorShieldDriverContainer(i2cAddress);

    if (driver == NULL)
    {
        Error::reportError_NoServoOnPin();
        return;
    }

    driver->registeredCount--;

    if (driver->registeredCount <= 0)
    {
        motorShieldDriverContainers.remove(driver);
        delete driver;
    }
}
