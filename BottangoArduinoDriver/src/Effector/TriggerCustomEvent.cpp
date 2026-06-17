#include "TriggerCustomEvent.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "Errors.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
TriggerCustomEvent::TriggerCustomEvent(char *identifier, byte pin, bool fireIsHigh) : AbstractEffector(0, 1), pin(pin), fireIsHigh(fireIsHigh)
{
    strcpy(myIdentifier, identifier);
    if (pin != 255)
    {

#ifdef NAMED_BOARD
        if (pin == 0)
        {
            Error::reportError_InvalidPin();
            return;
        }
#endif

#ifdef PIN_REMAPPING
        for (int i = 0; i < PIN_REMAP_LENGTH; i++)
        {
            if (inputPins[i] == pin)
            {
                pin = onboardPins[i];
                this->pin = pin;
                break;
            }
        }
#endif
        pinMode(pin, OUTPUT);
        if (fireIsHigh)
        {
            digitalWrite(pin, LOW);
        }
        else
        {
            digitalWrite(pin, HIGH);
        }
    }
    Callbacks::onEffectorRegistered(this);
}

void TriggerCustomEvent::updateOnLoop()
{

    unsigned long currentTime = Time::getCurrentTimeInMs();

    // Pointer to hold whichever TriggerCurve should fire next
    TriggerCurve *targetCurve = NULL;

    // Iterate over all possible curves stored in this effector
    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        TriggerCurve *iteratorCurve = (TriggerCurve *)curves[i];

        if (iteratorCurve == NULL)
        {
            continue;
        }

        // If the iterator curve's start time has already passed or is exactly now
        if (iteratorCurve->startTimeInMs <= currentTime)
        {
            // Choose the most recently scheduled curve (the one that started latest,
            // but not in the future) as the active target
            if (targetCurve == NULL || iteratorCurve->startTimeInMs > targetCurve->startTimeInMs)
            {
                targetCurve = iteratorCurve;
            }
        }
    }

    // If no curves were in progress, go to the final known state
    if (targetCurve != NULL && targetCurve->consumed == false)
    {
        shouldFire = true;
        targetCurve->consumed = true;
    }
}

void TriggerCustomEvent::driveOnLoop()
{
    if (shouldFire)
    {
        if (pin != 255)
        {
            if (fireIsHigh)
            {
                digitalWrite(pin, HIGH);
            }
            else
            {
                digitalWrite(pin, LOW);
            }
            pinOn = true;
            disablePinTime = Time::getCurrentTimeInMs() + TRIGGER_EVENT_PIN_TIME;
        }
        shouldFire = false;

        AbstractEffector::driveOnLoop();
        Callbacks::onTriggerCustomEventTriggered(this);
        AbstractEffector::callbackOnDriveComplete(1, true);
    }
    else
    {
        if (pin != 255 && pinOn && Time::getCurrentTimeInMs() >= disablePinTime)
        {
            if (fireIsHigh)
            {
                digitalWrite(pin, LOW);
            }
            else
            {
                digitalWrite(pin, HIGH);
            }
            pinOn = false;
        }
        AbstractEffector::callbackOnDriveComplete(0, false);
    }
}

void TriggerCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}