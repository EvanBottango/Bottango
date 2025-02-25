#include "Adafruit_PWMServoDriverContainer.h"

Adafruit_PwmServoDriverContainer::Adafruit_PwmServoDriverContainer(byte i2cAddress)
{
    this->i2cAddress = i2cAddress;
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    #ifdef I2C_CUSTOM_PINS
        Wire.begin(I2C_SDA, I2C_SCL);
        driver = new Adafruit_PWMServoDriver(i2cAddress, Wire);
    #else
        driver = new Adafruit_PWMServoDriver(i2cAddress);
    #endif
    driver->begin();
    Wire.setClock(400000);
    driver->setPWMFreq(50);
#endif
}

Adafruit_PwmServoDriverContainer::~Adafruit_PwmServoDriverContainer()
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    delete driver;
#endif
}