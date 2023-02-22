
#ifndef BOTTANGOARDUINO_FIXEDBEZIERCURVE_H
#define BOTTANGOARDUINO_FIXEDBEZIERCURVE_H

#include "BezierCurve.h"

class FixedBezierCurve : public BezierCurve
{
public:
    FixedBezierCurve(
        unsigned long startTimeInMs,
        unsigned long duration,
        long startY,
        long startControlX,
        long startControlY,
        long endY,
        long endControlX,
        long endControlY);

    /** returns a value in the range [startPosition - endPosition] */
    virtual float getValue(unsigned long currentTimeMs);

    virtual bool isInProgress(unsigned long currentTimeMs);

    virtual void dump();

    virtual unsigned long getEndTimeMs();

    virtual unsigned long getStartTimeMs();

    virtual float getStartMovement();
    virtual float getEndMovement();

private:
    const int PRECISION = 12;
    long lerpFixed(long start, long end, unsigned int u);
    unsigned long lerpFixedUnsigned(unsigned long start, unsigned long end, unsigned int u);

    void EvaluateForUX(unsigned int u, unsigned long &outx);
    void EvaluateForUY(unsigned int u, unsigned long &outy);

    unsigned long EvaluateFixed(unsigned long x);

    unsigned long curveStartTimeInMs = 0;
    unsigned long duration = 0;

    long startY = 0;
    long startControlX = 0;
    long startControlY = 0;

    long endY = 0;
    long endControlX = 0;
    long endControlY = 0;

    unsigned int lastU = 1 << (PRECISION - 1);
};

#endif // BOTTANGOARDUINO_FIXEDBEZIERCURVE_H