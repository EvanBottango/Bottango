#ifndef PinServoEffector_h
#define PinServoEffector_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"
#ifdef ESP32
#include <ESP32Servo.h>
#elif defined(CORE_TEENSY) && defined(USE_TEENSYPWM)
#include <PWMServo.h>
#else
#include "Servo.h"
#endif

class PinServoEffector : public LoopDrivenEffector
{
public:
    PinServoEffector(byte pin, short minPWM, short maxPWM, int maxPWMSec, short startPWM);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void destroy(bool systemShutdown) override;

protected:
private:
    byte pin = 0;
#if defined(CORE_TEENSY) && defined(USE_TEENSYPWM)
    PWMServo servo;
#else
    Servo servo;
#endif
};

#endif