#include "OnOffCustomEvent.h"
#include "Log.h"
#include "OnOffCurve.h"
#include "Time.h"

OnOffCustomEvent::OnOffCustomEvent(char *identifier, bool startOn) : AbstractEffector(0, 1)
{
    strcpy(myIdentifier, identifier);

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
    }
    Callbacks::effectorSignalOnLoop(this, currentOn);
}

void OnOffCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void OnOffCustomEvent::dump()
{
    LOG_LN(F("= On Off Event DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}