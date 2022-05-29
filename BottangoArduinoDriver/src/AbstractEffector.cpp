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

void AbstractEffector::setSync(int syncValue, bool isTracked)
{
}

void AbstractEffector::interruptTick()
{
}

void AbstractEffector::driveOnInterrupt(bool forward)
{
}

void AbstractEffector::updateOnLoop()
{
}

void AbstractEffector::driveOnLoop()
{
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

void AbstractEffector::dump()
{
}

void AbstractEffector::destroy()
{
    stop();
    clearCurves();

    Callbacks::onEffectorDeregistered(this);
}

int AbstractEffector::lerpSignal(float movement)
{
    float mapped = ((maxSignal - minSignal) * movement + minSignal);
    int mappedInt = (int)round(mapped);

    if (mappedInt > maxSignal)
    {
        mappedInt = maxSignal;
    }
    if (mappedInt < minSignal)
    {
        mappedInt = minSignal;
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
    drive = false;
}

AbstractEffector::~AbstractEffector()
{
}
