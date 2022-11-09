
#ifndef BOTTANGOARDUINO_COLORCURVE_H
#define BOTTANGOARDUINO_COLORCURVE_H

#include "Curve.h"
#include "Color.h"

class ColorCurve : public Curve
{
public:
    ColorCurve(unsigned long startTimeInMs, unsigned long duration, byte startR, byte startG, byte startB, byte endR, byte endG, byte endB);

    Color getValue(unsigned long currentTimeMs);

    bool isInProgress(unsigned long currentTimeMs);

    unsigned long getEndTimeMs();

    unsigned long getStartTimeMs();

    Color getStartColor();

    Color getEndColor();

private:
    unsigned long curveStartTimeInMs = 0;
    unsigned long duration = 0;

    Color startColor;
    Color endColor;
};

#endif //BOTTANGOARDUINO_COLORCURVE_H