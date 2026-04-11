
#ifndef BOTTANGOARDUINO_FLOATBEZIERCURVE_H
#define BOTTANGOARDUINO_FLOATBEZIERCURVE_H

#include "BezierCurve.h"

// vtable = 12 byte
// size of this class in RAM = 28 bytes (not including vtable pointer)
// Formula for RAM usage: n * instance size + vtable size, where n is the number of instances. So for example, 8 instances would be 8 * 28 + 12 = 236 bytes of RAM.
// getValue() needs ~90 bytes of RAM on the stack. After optimizations, it is down to ~56 byte of RAM on the stack. (38% reduction)
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
    virtual float getValue(unsigned long currentTimeMs) override;

    virtual bool isInProgress(unsigned long currentTimeMs) override;

    virtual unsigned long getEndTimeMs() override;

    virtual unsigned long getStartTimeMs() override;

    virtual float getStartMovement() override;
    virtual float getEndMovement() override;

private:
    //float lerp(float start, float end, float u);
	// This removes 18 bytes of RAM on stack
	inline float lerp(float start, float end, float u)
	{
		return ((end - start) * u) + start;
	}

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
