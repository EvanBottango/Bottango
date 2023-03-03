#include "FixedBezierCurve.h"
#include "Arduino.h"
#include "Log.h"
#include "../BottangoArduinoConfig.h"

// Returns roughly the same value as a FloatBezierCurve.cpp, but in fixed point math!
// Big thanks to @Dschadu for the smart thinking and advice along the way to bringing fixed point curve evaluation to Bottango!

FixedBezierCurve::FixedBezierCurve(
    unsigned long startTimeInMs,
    unsigned long duration,
    long startY,
    long startControlX,
    long startControlY,
    long endY,
    long endControlX,
    long endControlY) : BezierCurve()
{
    this->curveStartTimeInMs = startTimeInMs;
    this->duration = duration << PRECISION;

    this->startY = startY << PRECISION;

    // control y start/end changed to absolute, not relative
    // control x start/end kept relative, because start time is factored out of evaluate call

    this->startControlX = startControlX << PRECISION;
    this->startControlY = (startY + startControlY) << PRECISION;

    this->endY = endY << PRECISION;

    // edge case where final endControlX position is less than 0
    if (startTimeInMs + duration + endControlX < 0)
    {
        // ex
        // start1000 + duration500 + control-2000
        // change from -2000 (which would go 500 under 0)
        // to -1500 (which is the furthest the control can be and keep an evaluated positive position)
        // now
        // start1000 + duration500 + control-1500
        // this only should matter if start time is very low number, so very edge case-y
        this->endControlX = ((startTimeInMs + duration) * -1) << PRECISION;
    }
    else
    {
        this->endControlX = endControlX << PRECISION;
    }
    this->endControlY = (endY + endControlY) << PRECISION;
    this->lastU = 2;
}

unsigned long FixedBezierCurve::getEndTimeMs()
{
    return (unsigned long)(curveStartTimeInMs + (duration >> PRECISION));
}

unsigned long FixedBezierCurve::getStartTimeMs()
{
    return (unsigned long)(curveStartTimeInMs);
}

float FixedBezierCurve::getValue(unsigned long currentTimeMs)
{
    unsigned long scaledResult = EvaluateFixed((currentTimeMs - getStartTimeMs()) << PRECISION);
    unsigned long minimzedResult = scaledResult / COMPRESSED_SIGNAL_MAX_INT;
    float movementResult = (float)minimzedResult / (1 << PRECISION);

    return movementResult;
}

unsigned long FixedBezierCurve::EvaluateFixed(unsigned long x)
{
    unsigned int u = lastU;
    unsigned int uLower = 0;
    unsigned int uUpper = 1 << PRECISION;

    bool bailout = false;

    unsigned long evaluatedX = 0;
    unsigned long evaluatedY = 0; // only has signed long precision, but is clamped to 0 at the end. negative final evaluation is not a valid result, but need to keep the signed space for evaluation.

    while (true)
    {
        EvaluateForUX(u, evaluatedX);

        unsigned long delta = 0;
        if (evaluatedX > x)
        {
            delta = evaluatedX - x;
        }
        else
        {
            delta = x - evaluatedX;
        }

        if (delta < (unsigned int)(1 << PRECISION) || bailout)
        {
            EvaluateForUY(u, evaluatedY);
            lastU = u + 1;
            return evaluatedY;
        }
        else if (evaluatedX > x)
        {
            uUpper = u;
        }
        else if (evaluatedX < x)
        {
            uLower = u;
        }

        u = (uUpper - uLower) / 2 + uLower;

        if (uUpper - uLower <= 1)
        {
            bailout = true;
        }
    }
}

void FixedBezierCurve::EvaluateForUX(unsigned int u, unsigned long &outx)
{
    // note: evaluate factors out start Time, so just the relative used for these lerps to have a few less additions

    unsigned long p11x = (startControlX >> PRECISION) * u; // lerp from start -> start + start control, but start is factored out, so just (start control * u) since start is 0

    unsigned long p12x = lerpFixedUnsigned(startControlX, duration + endControlX, u); // lerp from st1000 + cx100 -> st1000 + dur500 + ecx-500 (negative case caught in constructor)

    unsigned long p13x = lerpFixedUnsigned(duration + endControlX, duration, u); // lerp from st1000 + dur500 + ecx-500 (negative case caught in constructor) -> st1000 + dur500

    unsigned long p21x = lerpFixedUnsigned(p11x, p12x, u); // all unsigned

    unsigned long p22x = lerpFixedUnsigned(p12x, p13x, u); // all unsigned

    outx = lerpFixedUnsigned(p21x, p22x, u); // all unsigned
}

void FixedBezierCurve::EvaluateForUY(unsigned int u, unsigned long &outy)
{
    // note: controls changed to absolute in constructor, so less additions

    long p11y = lerpFixed(startY, startControlY, u); // lerp from sty500 -> sty500 + stcy-600 (Signed)

    long p12y = lerpFixed(startControlY, endControlY, u); // lerp from sty500 + stcy-600 -> endy1000 + endcy500 (signed)

    long p13y = lerpFixed(endControlY, endY, u); // lerp from endy1000 + endcy500 => endy1000 (signed)

    long p21y = lerpFixed(p11y, p12y, u); // all signed

    long p22y = lerpFixed(p12y, p13y, u); // all signed

    long signedResult = lerpFixed(p21y, p22y, u); // all signed
    if (signedResult < 0)
    {
        outy = 0;
    }
    else
    {
        outy = (unsigned long)signedResult;
    }
}

long FixedBezierCurve::lerpFixed(long start, long end, unsigned int u)
{
    long result = (((end - start) >> PRECISION) * u) + start;
    return result;
}

unsigned long FixedBezierCurve::lerpFixedUnsigned(unsigned long start, unsigned long end, unsigned int u)
{
    unsigned long result;
    if (end < start)
    {
        result = (((start - end) >> PRECISION) * ((1 << PRECISION) - u)) + end;
    }
    else
    {
        result = (((end - start) >> PRECISION) * u) + start;
    }
    return result;
}

bool FixedBezierCurve::isInProgress(unsigned long currentTimeMs)
{
    return currentTimeMs >= getStartTimeMs() && currentTimeMs <= getEndTimeMs();
}

float FixedBezierCurve::getStartMovement()
{
    return (startY >> PRECISION) / COMPRESSED_SIGNAL_MAX;
}
float FixedBezierCurve::getEndMovement()
{
    return (endY >> PRECISION) / COMPRESSED_SIGNAL_MAX;
}

void FixedBezierCurve::dump()
{
    LOG(F("FIXCURVE[t=("))
    LOG_MKBUF
    LOG_FLOAT(getStartTimeMs())
    LOG(F(","))
    LOG_FLOAT(startY)
    LOG(F(")/("))
    LOG_FLOAT(startControlX)
    LOG(F(","))
    LOG_FLOAT(startControlY)
    LOG(F(")-("))
    LOG_FLOAT(getEndTimeMs())
    LOG(F(","))
    LOG_FLOAT(endY)
    LOG(F(")/("))
    LOG_FLOAT(endControlX)
    LOG(F(","))
    LOG_FLOAT(endControlY)
    LOG_LN(F(")"))
}