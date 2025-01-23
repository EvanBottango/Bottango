#include "TriggerCustomEvent.h"
#include "Log.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
TriggerCustomEvent::TriggerCustomEvent(char *identifier, byte pin, bool fireIsHigh) : AbstractEffector(0, 1), pin(pin), fireIsHigh(fireIsHigh)
{
    strcpy(myIdentifier, identifier);
    if (pin != 255)
    {
        pinMode(pin, OUTPUT);
        if (fireIsHigh)
        {
            digitalWrite(pin, false);
        }
        else
        {
            digitalWrite(pin, true);
        }
    }
    Callbacks::onEffectorRegistered(this);
}

void TriggerCustomEvent::updateOnLoop()
{
    unsigned long currentTime = Time::getCurrentTimeInMs();
    TriggerCurve *targetCurve = NULL;

    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        TriggerCurve *curve = (TriggerCurve *)curves[i];
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
                digitalWrite(pin, true);
            }
            else
            {
                digitalWrite(pin, false);
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
                digitalWrite(pin, false);
            }
            else
            {
                digitalWrite(pin, true);
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