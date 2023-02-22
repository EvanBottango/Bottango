
#ifndef BOTTANGOARDUINO_BEZIERCURVE_H
#define BOTTANGOARDUINO_BEZIERCURVE_H

#include "Curve.h"

class BezierCurve : public Curve
{
public:
    BezierCurve();

    /** returns a value in the range [startPosition - endPosition] */
    virtual float getValue(unsigned long currentTimeMs) = 0;

    virtual bool isInProgress(unsigned long currentTimeMs) = 0;

    virtual void dump() = 0;

    virtual unsigned long getEndTimeMs() = 0;

    virtual unsigned long getStartTimeMs() = 0;

    virtual float getStartMovement() = 0;
    virtual float getEndMovement() = 0;
};

#endif // BOTTANGOARDUINO_BEZIERCURVE_H
