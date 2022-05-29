#include "Adafruit_MotorShieldV2DriverContainer.h"

Adafruit_MotorShieldV2DriverContainer::Adafruit_MotorShieldV2DriverContainer(byte i2cAddress)
{
    this->i2cAddress = i2cAddress;
#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
    driver = new Adafruit_MotorShield(i2cAddress);
    driver->begin();
    Wire.setClock(400000);
#endif
}

Adafruit_MotorShieldV2DriverContainer::~Adafruit_MotorShieldV2DriverContainer()
{
#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
    delete driver;
#endif
}
