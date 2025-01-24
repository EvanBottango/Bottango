#ifndef PinStepperEffector_h
#define PinStepperEffector_h

#include "VelocityEffector.h"
#include "Arduino.h"

class PinStepperEffector : public VelocityEffector
{
public:
    PinStepperEffector(byte pin0, byte pin1, byte pin2, byte pin3, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxStepsPerSec, int startingStepsOffset);
    virtual void driveOnLoop() override;
    virtual void getIdentifier(char *outArray, short arraySize) override;

protected:
    byte pin0 = 0;
    byte pin1 = 0;
    byte pin2 = 0;
    byte pin3 = 0;

    byte stepLoop = 0;

private:
    void pulse();
};

#endif