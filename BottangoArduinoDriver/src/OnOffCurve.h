
#ifndef BOTTANGOARDUINO_ONOFFCURVE_H
#define BOTTANGOARDUINO_ONOFFCURVE_H

#include "Curve.h"

class OnOffCurve : public Curve
{
public:
    OnOffCurve(unsigned long startTimeInMs, bool on);
    unsigned long startTimeInMs = 0;
    bool on;
};

#endif //BOTTANGOARDUINO_ONOFFCURVE_H