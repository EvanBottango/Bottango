#include "PinStepperEffector.h"
#include "BottangoCore.h"
#include "Errors.h"

PinStepperEffector::PinStepperEffector(byte pin0, byte pin1, byte pin2, byte pin3, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignal) : VelocityEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignal)
{

#ifdef NAMED_BOARD
    if (pin0 == 0 || pin1 == 0 || pin2 == 0 || pin3 == 0)
    {
        Error::reportError_InvalidPin();
        return;
    }
#endif

#ifdef PIN_REMAPPING
    for (int i = 0; i < PIN_REMAP_LENGTH; i++)
    {
        if (pin0 == inputPins[i])
        {
            originalPin0 = pin0;
            pin0 = onboardPins[i];
        }
        else if (pin1 == inputPins[i])
        {
            pin1 = onboardPins[i];
        }
        else if (pin2 == inputPins[i])
        {
            pin2 = onboardPins[i];
        }
        else if (pin3 == inputPins[i])
        {
            pin3 = onboardPins[i];
        }
    }
#endif

    this->pin0 = pin0;
    this->pin1 = pin1;
    this->pin2 = pin2;
    this->pin3 = pin3;

    digitalWrite(pin0, LOW);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    digitalWrite(pin3, LOW);

    pinMode(pin0, OUTPUT);
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);

    Callbacks::onEffectorRegistered(this);
}

void PinStepperEffector::driveOnLoop()
{
    bool didChange = false;
    if (drive > 0)
    {
        if (stepLoop == 3)
        {
            stepLoop = 0;
        }
        else
        {
            stepLoop++;
        }
        pulse();
        if (sync != 0 || autoSync != 0)
        {
            updateSync(drive);
        }
        else
        {
            currentSignal++;
            didChange = true;
        }
        drive = 0;
    }
    else if (drive < 0)
    {
        if (stepLoop == 0)
        {
            stepLoop = 3;
        }
        else
        {
            stepLoop--;
        }
        pulse();
        if (sync != 0 || autoSync != 0)
        {
            updateSync(drive);
        }
        else
        {
            currentSignal--;
            didChange = true;
        }
        drive = 0;
    }
    VelocityEffector::driveOnLoop();
    AbstractEffector::callbackOnDriveComplete(currentSignal, didChange);
}

void PinStepperEffector::pulse()
{
    switch (stepLoop)
    {
    case 0:
        digitalWrite(pin0, HIGH);
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        digitalWrite(pin3, LOW);
        break;
    case 1:
        digitalWrite(pin0, LOW);
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, HIGH);
        digitalWrite(pin3, LOW);
        break;
    case 2:
        digitalWrite(pin0, LOW);
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        digitalWrite(pin3, HIGH);
        break;
    case 3:
        digitalWrite(pin0, HIGH);
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
        digitalWrite(pin3, HIGH);
        break;
    }
}

void PinStepperEffector::getIdentifier(char *outArray, short arraySize)
{
#ifdef PIN_REMAPPING
    snprintf(outArray, arraySize, "%d", (int)originalPin0);
#else
    snprintf(outArray, arraySize, "%d", (int)pin0);
#endif
}
