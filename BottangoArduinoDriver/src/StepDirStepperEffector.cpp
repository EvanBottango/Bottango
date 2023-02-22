#include "StepDirStepperEffector.h"
#include "Log.h"
#include "BottangoCore.h"

StepDirStepperEffector::StepDirStepperEffector(byte stepPin, byte dirPin, bool clockwiseIsLow, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignalOffset) : VelocityEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignalOffset)
{
    this->stepPin = stepPin;
    this->dirPin = dirPin;

    this->clockwiseIsLow = clockwiseIsLow;

    this->pulseStartTimeUs = 0;

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

void StepDirStepperEffector::driveOnLoop()
{
    unsigned long nowUS = micros();

    if (pulseStartTimeUs > 0)
    {
        if (nowUS - pulseStartTimeUs > minPulseWidthUs)
        {
            digitalWrite(stepPin, LOW);
            pulseStartTimeUs = 0;
            drive = 0;
        }
        else
        {
            return;
        }
    }

    if (drive > 0)
    {
        if (!currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? LOW : HIGH);
            currDirectionIsClockwise = true;
        }

        digitalWrite(stepPin, HIGH);

        pulseStartTimeUs = nowUS;
        if (sync != 0)
        {
            if (sync < -100)
            {
                if (Callbacks::isStepperAutoHomeComplete(this))
                {
                    endAutoSync();
                }
            }
            else
            {
                sync++;
            }
        }
        else
        {
            currentSignal++;
        }
    }
    else if (drive < 0)
    {
        if (currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? HIGH : LOW);
            currDirectionIsClockwise = false;
        }

        digitalWrite(stepPin, HIGH);

        pulseStartTimeUs = nowUS;

        if (sync != 0)
        {
            if (sync > 100)
            {
                if (Callbacks::isStepperAutoHomeComplete(this))
                {
                    endAutoSync();
                }
            }
            else
            {
                sync--;
            }
        }
        else
        {
            currentSignal--;
        }
    }
    VelocityEffector::driveOnLoop();
}

void StepDirStepperEffector::getIdentifier(char *outArray, short arraySize)
{
    snprintf(outArray, arraySize, "%d", (int)stepPin);
}
