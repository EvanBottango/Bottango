
#ifndef BOTTANGOARDUINO_TRIGGERCURVE_H
#define BOTTANGOARDUINO_TRIGGERCURVE_H

#include "Curve.h"

class TriggerCurve : public Curve
{
public:
    TriggerCurve(unsigned long startTimeInMs);
    unsigned long startTimeInMs = 0;
    bool consumed = false;
};

#endif //BOTTANGOARDUINO_TRIGGERCURVE_H