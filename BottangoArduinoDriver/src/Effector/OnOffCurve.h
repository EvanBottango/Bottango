#pragma once

#include "Curve.h"

class OnOffCurve : public Curve
{
public:
    OnOffCurve(unsigned long startTimeInMs, bool on);
    unsigned long startTimeInMs = 0;
    bool on = false;
    virtual unsigned long getStartTimeMs();
};