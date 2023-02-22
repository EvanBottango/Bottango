
#ifndef BOTTANGOARDUINO_FLOATBEZIERCURVE_H
#define BOTTANGOARDUINO_FLOATBEZIERCURVE_H

#include "BezierCurve.h"

class FloatBezierCurve : public BezierCurve
{
public:
    FloatBezierCurve(
        unsigned long startTimeInMs,
        long duration,
        int startY,
        long startControlX,
        int startControlY,
        int endY,
        long endControlX,
        int endControlY);

    /** returns a value in the range [startPosition - endPosition] */
    virtual float getValue(unsigned long currentTimeMs);

    virtual bool isInProgress(unsigned long currentTimeMs);

    virtual void dump();

    virtual unsigned long getEndTimeMs();

    virtual unsigned long getStartTimeMs();

    virtual float getStartMovement();
    virtual float getEndMovement();

private:
    float lerp(float start, float end, float u);

    void EvaluateForUX(float u, float &outx);
    void EvaluateForUY(float u, float &outy);

    float Evaluate(unsigned long x);

    unsigned long curveStartTimeInMs = 0;
    long duration = 0;
    float lastU = 0.5f;

    int startY = 0;
    long startControlX = 0;
    int startControlY = 0;

    int endY = 0;
    long endControlX = 0;
    int endControlY = 0;
};

#endif // BOTTANGOARDUINO_FLOATBEZIERCURVE_H
