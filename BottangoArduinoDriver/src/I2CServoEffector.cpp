#include "I2CServoEffector.h"
#include "Errors.h"
#include "I2CPool.h"

I2CServoEffector::I2CServoEffector(byte i2cAddress, byte pin, short minPWM, short maxPWM, int maxPWMSec, short startSignal) : LoopDrivenEffector(minPWM, maxPWM, maxPWMSec, startSignal)
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY
    this->pin = pin;
    this->i2cAddress = i2cAddress;

    registerPWMDriverEffector(i2cAddress);
    this->driver = getPWMDriverContainer(i2cAddress)->driver;

    LOG_MKBUF
    LOG(F("Attach i2c Servo address "))
    LOG_INT(i2cAddress);
    LOG(F(" pin"))
    LOG_INT(pin)
    LOG_NEWLINE()

    this->driver->writeMicroseconds(pin, startSignal);
    currentSignal = startSignal;

#endif

    Callbacks::onEffectorRegistered(this);
}

void I2CServoEffector::driveOnLoop()
{
    if (currentSignal != targetSignal)
    {
#ifdef USE_ADAFRUIT_PWM_LIBRARY
        driver->writeMicroseconds(pin, currentSignal);
#endif
        currentSignal = targetSignal;
    }

    LoopDrivenEffector::driveOnLoop();
}

void I2CServoEffector::getIdentifier(char *outArray, short arraySize)
{
    char addressSegment[4];
    char pinSegment[2];

    sprintf(addressSegment, "%d", (int)i2cAddress);
    sprintf(pinSegment, "%d", pin);

    strcpy(outArray, addressSegment);
    strcat(outArray, pinSegment);
}

void I2CServoEffector::destroy()
{
    removePWMDriverEffector(i2cAddress);
    AbstractEffector::destroy();
}

void I2CServoEffector::dump()
{
    LOG_LN(F("= SERVO DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}