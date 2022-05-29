#include "StepDirStepperEffector.h"
#include "Log.h"
#include "BottangoCore.h"

StepDirStepperEffector::StepDirStepperEffector(byte stepPin, byte dirPin, bool clockwiseIsLow, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignalOffset) : InterruptDrivenEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignalOffset)
{
    this->stepPin = stepPin;
    this->dirPin = dirPin;

    this->clockwiseIsLow = clockwiseIsLow;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, clockwiseIsLow ? LOW : HIGH);

    currDirectionIsClockwise = true;

    LOG_MKBUF
    LOG(F("Attaching stepdir stepper. step "))
    LOG_INT(stepPin)
    LOG(F(" dir, "))
    LOG_INT(dirPin)
    LOG_NEWLINE()

    Callbacks::onEffectorRegistered(this);
}

void StepDirStepperEffector::driveOnInterrupt(bool forward)
{
    if (forward)
    {
        if (!currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? LOW : HIGH);
            currDirectionIsClockwise = true;
        }
    }
    else
    {
        if (currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? HIGH : LOW);
            currDirectionIsClockwise = false;
        }
    }

    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin, LOW);
}

void StepDirStepperEffector::getIdentifier(char *outArray, short arraySize)
{
    snprintf(outArray, arraySize, "%d", (int)stepPin);
}
