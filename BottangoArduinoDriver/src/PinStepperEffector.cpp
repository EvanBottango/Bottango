#include "PinStepperEffector.h"
#include "Log.h"
#include "BottangoCore.h"

PinStepperEffector::PinStepperEffector(byte pin0, byte pin1, byte pin2, byte pin3, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignal) : InterruptDrivenEffector(maxCounterClockwiseSteps, maxClockwiseSteps, maxSignalPerSec, startingSignal)
{
    this->pin0 = pin0;
    this->pin1 = pin1;
    this->pin2 = pin2;
    this->pin3 = pin3;

    pinMode(pin0, OUTPUT);
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);

    digitalWrite(pin0, LOW);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    digitalWrite(pin3, LOW);

    LOG_MKBUF
    LOG(F("Attaching pin stepper "))
    LOG_INT(pin0)
    LOG(F(", "))
    LOG_INT(pin1)
    LOG(F(", "))
    LOG_INT(pin2)
    LOG(F(", "))
    LOG_INT(pin3)
    LOG_NEWLINE()

    Callbacks::onEffectorRegistered(this);
}

void PinStepperEffector::driveOnInterrupt(bool forward)
{
    if (forward)
    {
        if (stepLoop == 3)
        {
            stepLoop = 0;
        }
        else
        {
            stepLoop++;
        }
    }
    else
    {
        if (stepLoop == 0)
        {
            stepLoop = 3;
        }
        else
        {
            stepLoop--;
        }
    }

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
    snprintf(outArray, arraySize, "%d", (int)pin0);
}
