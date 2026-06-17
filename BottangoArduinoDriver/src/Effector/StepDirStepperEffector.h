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

#ifdef PIN_REMAPPING
    byte originalStepPin = 0;
#endif

    bool clockwiseIsLow = false;           // direction of low signal, using packed bool in abstract effector
    bool currDirectionIsClockwise = false; // last set direction, using packed bool in abstract effector

    unsigned long holdStartTimeUs;
    bool stepHigh = false;
    bool dirSwitch = false;
    const byte minPulseWidthSide = 5;
    const byte dirPulseHold = 2;

private:
    void stepDrive();
};

#endif