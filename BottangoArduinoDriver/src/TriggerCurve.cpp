#include "TriggerCurve.h"

TriggerCurve::TriggerCurve(unsigned long startTimeInMs) : Curve()
{
    this->startTimeInMs = startTimeInMs;
    consumed = false;
};