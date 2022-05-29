#include "I2CStepperEffector.h"
#include "Errors.h"
#include "I2CPool.h"

I2CStepperEffector::I2CStepperEffector(byte i2cAddress, byte pin, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignal) : LoopDrivenEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignal)
{
#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
    this->pin = pin;
    this->i2cAddress = i2cAddress;
    this->syncValue = 0;

    registerMotorShieldDriverEffector(i2cAddress);
    this->stepper = getMotorShieldDriverContainer(i2cAddress)->driver->getStepper(200, pin); // steps per rev don't matter, we're not using it. So 200 default value is fine.

    LOG_MKBUF
    LOG(F("Attach i2c Stepper address "))
    LOG_INT(i2cAddress);
    LOG(F(" pin"))
    LOG_INT(pin)
    LOG_NEWLINE()

#endif

    Callbacks::onEffectorRegistered(this);
}

void I2CStepperEffector::getIdentifier(char *outArray, short arraySize)
{
    char addressSegment[4];
    char pinSegment[2];

    sprintf(addressSegment, "%d", (int)i2cAddress);
    sprintf(pinSegment, "%d", pin);

    strcpy(outArray, addressSegment);
    strcat(outArray, pinSegment);
}

void I2CStepperEffector::destroy()
{
    removeMotorShieldDriverEffector(i2cAddress);
    // I don't think delete ref to stepper, as that's managed by controller
    AbstractEffector::destroy();
}

void I2CStepperEffector::dump()
{
    LOG_LN(F("= STEPPER DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}

void I2CStepperEffector::driveOnLoop()
{
#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY

    if (syncValue > 0)
    {
        stepper->onestep(FORWARD, DOUBLE);
        syncValue--;
        return;
    }
    else if (syncValue < 0)
    {
        stepper->onestep(BACKWARD, DOUBLE);
        syncValue++;
        return;
    }

    if (currentSignal > targetSignal)
    {
        stepper->onestep(BACKWARD, DOUBLE);
        currentSignal--;
    }
    else if (currentSignal < targetSignal)
    {
        stepper->onestep(FORWARD, DOUBLE);
        currentSignal++;
    }

#endif

    LoopDrivenEffector::driveOnLoop();
}

void I2CStepperEffector::setSync(int syncValue, bool isTracked)
{
    if (isTracked)
    {
        return;
    }
    this->syncValue = syncValue;
}