#include "PinServoEffector.h"
#include "Log.h"
#include "../BottangoArduinoModules.h"

PinServoEffector::PinServoEffector(byte pin, short minPWM, short maxPWM, int maxPWMSec, short startSignal) : LoopDrivenEffector(minPWM, maxPWM, maxPWMSec, startSignal)
{
    this->pin = pin;

#ifdef PIN_REMAPPING
    bool matchFound = false;
    for (int i = 0; i < PIN_REMAP_LENGTH; i++)
    {
        if (inputPins[i] == pin)
        {
            servo.attach(onboardPins[i]);
            matchFound = true;
            break;
        }
    }
    if (!matchFound)
    {
        servo.attach(pin);
    }
#else
    servo.attach(pin);
#endif

#ifdef ESP32
    servo.setTimerWidth(16);
#endif
    servo.writeMicroseconds(startSignal);

    Callbacks::onEffectorRegistered(this);
}

void PinServoEffector::driveOnLoop()
{
    bool didChange = false;
    if (currentSignal != targetSignal)
    {
        servo.writeMicroseconds(targetSignal);
        currentSignal = targetSignal;
        didChange = true;
    }
    LoopDrivenEffector::driveOnLoop();
    AbstractEffector::callbackOnDriveComplete(currentSignal, didChange);
}

void PinServoEffector::getIdentifier(char *outArray, short arraySize)
{
    snprintf(outArray, arraySize, "%d", (int)pin);
}

void PinServoEffector::destroy(bool systemShutdown)
{
#ifdef ESP32
    // this more cleanly detaches servos when shutting down a lot at once
    // resulting in less random movement on detach
    servo.writeMicroseconds(servo.readMicroseconds());
    delay(25);
    servo.release();
    delay(25);
    servo.detach();
#endif

    AbstractEffector::destroy(systemShutdown);
}