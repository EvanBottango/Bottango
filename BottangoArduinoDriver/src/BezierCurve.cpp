
#include "BezierCurve.h"
#include "Arduino.h"
#include "Log.h"

BezierCurve::BezierCurve(
    unsigned long startTimeInMs,
    unsigned long duration,
    int startY,
    long startControlX,
    int startControlY,
    int endY,
    long endControlX,
    int endControlY) : Curve()
{

    this->curveStartTimeInMs = startTimeInMs;
    this->duration = duration;

    this->startY = startY;
    this->startControlX = startControlX;
    this->startControlY = startControlY;

    this->endY = endY;
    this->endControlX = endControlX;
    this->endControlY = endControlY;
}

float BezierCurve::getValue(unsigned long currentTimeMs)
{
    // 25,000 comes in
    // curve starts at 10,000 and has 50,000 duration

    return Evaluate(currentTimeMs - curveStartTimeInMs) * 0.001f;
}

unsigned long BezierCurve::getEndTimeMs()
{
    return curveStartTimeInMs + (unsigned long)duration;
}

unsigned long BezierCurve::getStartTimeMs()
{
    return curveStartTimeInMs;
}

float BezierCurve::Evaluate(unsigned long x)
{

    float uLower = 0;
    float uUpper = 1;
    float u = (uUpper - uLower) / 2 + uLower;

    while (true)
    {
        float evaluatedX = 0, evaluatedY = 0;
        EvaluateForU(u, evaluatedX, evaluatedY);

        if (abs(evaluatedX - x) < 1)
        {
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
    }
}

void BezierCurve::EvaluateForU(float u, float &outx, float &outy)
{

    float p11x = (float)startControlX * u;
    float p11y = lerp((float)startY, (float)(startY + startControlY), u);

    float p12x = lerp((float)startControlX, (float)(duration + endControlX), u);
    float p12y = lerp((float)(startY + startControlY), (float)(endY + endControlY), u);

    float p13x = lerp((float)(duration + endControlX), (float)duration, u);
    float p13y = lerp((float)(endY + endControlY), (float)endY, u);

    float p21x = lerp(p11x, p12x, u);
    float p21y = lerp(p11y, p12y, u);

    float p22x = lerp(p12x, p13x, u);
    float p22y = lerp(p12y, p13y, u);

    outx = lerp(p21x, p22x, u);
    outy = lerp(p21y, p22y, u);
}

float BezierCurve::lerp(float start, float end, float u)
{
    float result = ((end - start) * u) + start;
    return result;
}

bool BezierCurve::isInProgress(unsigned long currentTimeMs)
{
    return currentTimeMs >= getStartTimeMs() && currentTimeMs <= getEndTimeMs();
}

float BezierCurve::getStartMovement()
{
    return startY * 0.001f;
}
float BezierCurve::getEndMovement()
{
    return endY * 0.001f;
}

void BezierCurve::dump()
{
    LOG(F("CURVE[t=("))
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
