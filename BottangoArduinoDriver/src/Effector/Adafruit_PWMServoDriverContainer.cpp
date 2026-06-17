#include "Adafruit_PWMServoDriverContainer.h"

Adafruit_PwmServoDriverContainer::Adafruit_PwmServoDriverContainer(byte i2cAddress)
{
    this->i2cAddress = i2cAddress;
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    driver = new Adafruit_PWMServoDriver(i2cAddress);
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