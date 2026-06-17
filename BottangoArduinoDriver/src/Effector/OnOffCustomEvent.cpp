#include "OnOffCustomEvent.h"
#include "OnOffCurve.h"
#include "Time.h"
#include "Errors.h"

OnOffCustomEvent::OnOffCustomEvent(char *identifier, bool startOn, byte pin) : AbstractEffector(0, 1), pin(pin)
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
    }

    Callbacks::onEffectorRegistered(this);
}

void OnOffCustomEvent::updateOnLoop()
{
    unsigned long currentTime = Time::getCurrentTimeInMs();
    OnOffCurve *targetCurve = NULL;

    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        OnOffCurve *curve = (OnOffCurve *)curves[i];
        if (curve == NULL)
        {
            continue;
        }

        if (curve->startTimeInMs <= currentTime)
        {
            if (targetCurve == NULL || curve->startTimeInMs > targetCurve->startTimeInMs)
            {
                targetCurve = curve;
            }
        }
    }

    // If no curves were in progress, go to the final known state
    if (targetCurve != NULL)
    {
        targetOn = targetCurve->on;
    }
}

void OnOffCustomEvent::driveOnLoop()
{
    if (currentOn != targetOn)
    {
        // callback here
        currentOn = targetOn;
        Callbacks::onOnOffCustomEventOnOffChanged(this, currentOn != 0);
        if (pin != 255)
        {
            digitalWrite(pin, currentOn);
        }
        AbstractEffector::driveOnLoop();
        AbstractEffector::callbackOnDriveComplete(currentOn, true);
    }
    else
    {
        AbstractEffector::driveOnLoop();
        AbstractEffector::callbackOnDriveComplete(currentOn, false);
    }
}

void OnOffCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}