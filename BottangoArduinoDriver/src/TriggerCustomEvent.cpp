#include "TriggerCustomEvent.h"
#include "Log.h"
#include "TriggerCurve.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"

// Signal is 0 - 0, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
TriggerCustomEvent::TriggerCustomEvent(char *identifier, byte pin) : AbstractEffector(0, 1), pin(pin)
{
    strcpy(myIdentifier, identifier);
    if (pin != 255)
    {
        pinMode(pin, OUTPUT);
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
            digitalWrite(pin, true);
            pinOn = true;
            disablePinTime = Time::getCurrentTimeInMs() + TRIGGER_EVENT_PIN_TIME;
        }
        shouldFire = false;
        Callbacks::onTriggerCustomEventTriggered(this);
        Callbacks::effectorSignalOnLoop(this, 1);
    }
    else
    {
        if (pin != 255 && pinOn && Time::getCurrentTimeInMs() >= disablePinTime)
        {
            digitalWrite(pin, false);
            pinOn = false;
        }
        Callbacks::effectorSignalOnLoop(this, 0);
    }
}

void TriggerCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void TriggerCustomEvent::dump()
{
    LOG_LN(F("= Trigger Event DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}