#ifndef StepDirStepperEffector_h
#define StepDirStepperEffector_h

#include "VelocityEffector.h"
#include "Arduino.h"

class StepDirStepperEffector : public VelocityEffector
{
public:
    StepDirStepperEffector(byte stepPin, byte dirPin, bool clockwiseIsLow, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxStepsPerSec, int startingStepsOffset);

    virtual void driveOnLoop() override;
    virtual void getIdentifier(char *outArray, short arraySize) override;

protected:
    byte stepPin = 0;
    byte dirPin = 0;

    bool clockwiseIsLow = false;           // direction of low signal, using packed bool in abstract effector
    bool currDirectionIsClockwise = false; // last set direction, using packed bool in abstract effector

    unsigned long pulseStartTimeUs;
    const byte minPulseWidthUs = 10;

private:
    void stepDrive();
};

#endif