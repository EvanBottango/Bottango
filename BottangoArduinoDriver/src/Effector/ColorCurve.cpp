#include "ColorCurve.h"

ColorCurve::ColorCurve(unsigned long startTimeInMs, unsigned long duration, byte startR, byte startG, byte startB, byte endR, byte endG, byte endB) : Curve()
{

    this->curveStartTimeInMs = startTimeInMs;
    this->duration = duration;

    this->startColor.r = startR;
    this->startColor.g = startG;
    this->startColor.b = startB;

    this->endColor.r = endR;
    this->endColor.g = endG;
    this->endColor.b = endB;
}

unsigned long ColorCurve::getEndTimeMs()
{
    return curveStartTimeInMs + (unsigned long)duration;
}

unsigned long ColorCurve::getStartTimeMs()
{
    return curveStartTimeInMs;
}

bool ColorCurve::isInProgress(unsigned long currentTimeMs)
{
    return currentTimeMs >= getStartTimeMs() && currentTimeMs <= getEndTimeMs();
}

Color ColorCurve::getStartColor()
{
    return startColor;
}

Color ColorCurve::getEndColor()
{
    return endColor;
}

Color ColorCurve::getValue(unsigned long currentTimeMs)
{
    if (duration <= 0.0f)
    {
        return Color(endColor.r, endColor.g, endColor.b);
    }

    float u = ((float)currentTimeMs - (float)curveStartTimeInMs) / (float)duration;

    if (u <= 0.0f)
    {
        return Color(startColor.r, startColor.g, startColor.b);
    }
    else if (u >= 1.0f)
    {
        return Color(endColor.r, endColor.g, endColor.b);
    }

    byte r = round(((endColor.r - startColor.r) * u) + startColor.r);
    byte g = round(((endColor.g - startColor.g) * u) + startColor.g);
    byte b = round(((endColor.b - startColor.b) * u) + startColor.b);

    return Color(r, g, b);
}
