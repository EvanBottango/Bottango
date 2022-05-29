
#ifndef BOTTANGOARDUINO_BEZIERCURVE_H
#define BOTTANGOARDUINO_BEZIERCURVE_H

#include "Curve.h"

class BezierCurve : public Curve
{
public:
    BezierCurve(
        unsigned long startTimeInMs,
        unsigned long duration,
        int startY,
        long startControlX,
        int startControlY,
        int endY,
        long endControlX,
        int endControlY);

    /** returns a value in the range [startPosition - endPosition] */
    float getValue(unsigned long currentTimeMs);

    bool isInProgress(unsigned long currentTimeMs);

    void dump();

    unsigned long getEndTimeMs();

    unsigned long getStartTimeMs();

    float getStartMovement();
    float getEndMovement();

private:
    float lerp(float start, float end, float u);

    void EvaluateForU(float u, float &outx, float &outy);

    float Evaluate(unsigned long x);

    unsigned long curveStartTimeInMs = 0;
    unsigned long duration = 0;

    int startY = 0;
    long startControlX = 0;
    int startControlY = 0;

    int endY = 0;
    long endControlX = 0;
    int endControlY = 0;
};

#endif //BOTTANGOARDUINO_CURVE_H
