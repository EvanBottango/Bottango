#include "StepDirStepperEffector.h"
#include "BottangoCore.h"
#include "Errors.h"

StepDirStepperEffector::StepDirStepperEffector(byte stepPin, byte dirPin, bool clockwiseIsLow, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignalOffset) : VelocityEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignalOffset)
{

#ifdef NAMED_BOARD
    if (stepPin == 0 || dirPin == 0)
    {
        Error::reportError_InvalidPin();
        return;
    }
#endif

#ifdef PIN_REMAPPING
    for (int i = 0; i < PIN_REMAP_LENGTH; i++)
    {
        if (stepPin == inputPins[i])
        {
            originalStepPin = stepPin;
            stepPin = onboardPins[i];
        }
        else if (dirPin == inputPins[i])
        {
            dirPin = onboardPins[i];
        }
    }
#endif

    this->stepPin = stepPin;
    this->dirPin = dirPin;

    this->clockwiseIsLow = clockwiseIsLow;

    this->holdStartTimeUs = 0;
    this->stepHigh = false;
    this->dirSwitch = false;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, clockwiseIsLow ? LOW : HIGH);

    currDirectionIsClockwise = true;

    Callbacks::onEffectorRegistered(this);
}

void StepDirStepperEffector::driveOnLoop()
{
    VelocityEffector::driveOnLoop();

    unsigned long nowUS = micros();

    // need to hold before pulsing?
    if (holdStartTimeUs > 0)
    {
        // holding for a direction switch?
        if (dirSwitch)
        {
            // direction switch hold complete
            if (nowUS - holdStartTimeUs >= dirPulseHold)
            {
                dirSwitch = false;
                holdStartTimeUs = 0;
            }
            // wait
            else
            {
                return;
            }
        }
        // otherwise, holding for a pulse
        else
        {
            // ready to progress the pulse?
            if (nowUS - holdStartTimeUs >= minPulseWidthSide)
            {
                // was high, set low
                if (stepHigh)
                {
                    digitalWrite(stepPin, LOW);
                    holdStartTimeUs = nowUS;
                    stepHigh = false;
                    return;
                }
                // has been low long enough
                else
                {
                    holdStartTimeUs = 0;
                    drive = 0;
                }
            }
            // wait
            else
            {
                return;
            }
        }
    }

    bool didChange = false;
    // move clockwise
    if (drive > 0)
    {
        // need to swap dir pin and hold a tiny bit?
        if (!currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? LOW : HIGH);
            currDirectionIsClockwise = true;
            holdStartTimeUs = nowUS;
            dirSwitch = true;
            return;
        }

        // set pin high
        digitalWrite(stepPin, HIGH);
        holdStartTimeUs = nowUS;
        stepHigh = true;

        // update signal or sync value
        if (autoSync != 0 || sync != 0)
        {
            updateSync(drive);
        }
        else
        {
            didChange = true;
            currentSignal++;
        }
    }
    // move counterclockwise
    else if (drive < 0)
    {
        // need to swap dir pin and hold a tiny bit?
        if (currDirectionIsClockwise)
        {
            digitalWrite(dirPin, clockwiseIsLow ? HIGH : LOW);
            currDirectionIsClockwise = false;
            holdStartTimeUs = nowUS;
            dirSwitch = true;
            return;
        }

        // set pin high
        digitalWrite(stepPin, HIGH);
        holdStartTimeUs = nowUS;
        stepHigh = true;

        // update signal or sync value
        if (autoSync != 0 || sync != 0)
        {
            updateSync(drive);
        }
        else
        {
            didChange = true;
            currentSignal--;
        }
    }
    VelocityEffector::driveOnLoop();
    AbstractEffector::callbackOnDriveComplete(currentSignal, didChange);
}

void StepDirStepperEffector::getIdentifier(char *outArray, short arraySize)
{
#ifdef PIN_REMAPPING
    snprintf(outArray, arraySize, "%d", (int)originalStepPin);
#else
    snprintf(outArray, arraySize, "%d", (int)stepPin);
#endif
}