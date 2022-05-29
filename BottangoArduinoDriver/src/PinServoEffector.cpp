#include "PinServoEffector.h"
#include "Log.h"

PinServoEffector::PinServoEffector(byte pin, short minPWM, short maxPWM, int maxPWMSec, short startSignal) : LoopDrivenEffector(minPWM, maxPWM, maxPWMSec, startSignal)
{
    this->pin = pin;

    servo.attach(pin);
    servo.writeMicroseconds(startSignal);

    Callbacks::onEffectorRegistered(this);
}

void PinServoEffector::driveOnLoop()
{
    if (currentSignal != targetSignal)
    {
        servo.writeMicroseconds(targetSignal);
        currentSignal = targetSignal;
    }
    LoopDrivenEffector::driveOnLoop();
}

void PinServoEffector::getIdentifier(char *outArray, short arraySize)
{
    snprintf(outArray, arraySize, "%d", (int)pin);
}

void PinServoEffector::destroy()
{
    servo.detach();
    AbstractEffector::destroy();
}

void PinServoEffector::dump()
{
    LOG_LN(F("= SERVO DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}