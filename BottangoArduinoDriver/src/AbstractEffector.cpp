#include "AbstractEffector.h"
#include "Log.h"
#include "BottangoCore.h"

AbstractEffector::AbstractEffector(int minSignal, int maxSignal)
{
    this->minSignal = minSignal;
    this->maxSignal = maxSignal;

    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        curves[i] = NULL;
    }
}

void AbstractEffector::setSync(int syncValue)
{
}

void AbstractEffector::updateOnLoop()
{
}

void AbstractEffector::driveOnLoop()
{
}

void AbstractEffector::callbackOnDriveComplete(int currentSignal, bool didChange)
{
    Callbacks::effectorSignalOnLoop(this, currentSignal, didChange);
}

void AbstractEffector::updateSignalBounds(int minSignal, int maxSignal, int signalSpeed)
{
    this->minSignal = minSignal;
    this->maxSignal = maxSignal;
}

void AbstractEffector::addCurve(Curve *curve)
{

    if (curves[curvesIdx] != NULL)
    {
        free(curves[curvesIdx]);
    }

    curves[curvesIdx] = curve;

    curvesIdx = (curvesIdx + 1) % MAX_NUM_CURVES;
}

void AbstractEffector::destroy(bool systemShutdown)
{
    stop();
    clearCurves();

    Callbacks::onEffectorDeregistered(this);
}

int AbstractEffector::lerpSignal(float movement)
{
    float mapped = ((maxSignal - minSignal) * movement + minSignal);
    int mappedInt = (int)round(mapped);

    // account for inverted min/max signal (IE go from 2000 -> 1000)
    if (maxSignal > minSignal)
    {
        if (mappedInt > maxSignal)
        {
            mappedInt = maxSignal;
        }
        if (mappedInt < minSignal)
        {
            mappedInt = minSignal;
        }
    }
    else
    {
        if (mappedInt > minSignal)
        {
            mappedInt = minSignal;
        }
        if (mappedInt < maxSignal)
        {
            mappedInt = maxSignal;
        }
    }

    return mappedInt;
}

bool AbstractEffector::respondsToIdentifier(char *identifier)
{
    char myEffectorIdentifier[9];
    getIdentifier(myEffectorIdentifier, 9);

    return strcmp(identifier, myEffectorIdentifier) == 0;
}

void AbstractEffector::clearCurves()
{
    stop();
    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        free(curves[i]);
        curves[i] = NULL;
    }
}

void AbstractEffector::stop()
{
}

bool AbstractEffector::useFloatCurve()
{
#if defined(DEFAULT_FLOAT_CURVE)
    return true;
#elif defined(DEFAULT_FIXED_CURVE)
    return false;
#else
    retue true;
#endif
}

AbstractEffector::~AbstractEffector()
{
}
