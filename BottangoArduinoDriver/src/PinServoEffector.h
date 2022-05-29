#ifndef PinServoEffector_h
#define PinServoEffector_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"
#include "Servo.h"

class PinServoEffector : public LoopDrivenEffector
{
public:
    PinServoEffector(byte pin, short minPWM, short maxPWM, int maxPWMSec, short startPWM);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void destroy() override;
    virtual void dump() override;

protected:
private:
    byte pin = 0;
    Servo servo;
};

#endif