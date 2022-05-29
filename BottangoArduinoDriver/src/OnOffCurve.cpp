#include "OnOffCurve.h"

OnOffCurve::OnOffCurve(unsigned long startTimeInMs, bool on) : Curve()
{
    this->startTimeInMs = startTimeInMs;
    this->on = on;
};