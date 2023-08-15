#ifndef AbstractEffector_h
#define AbstractEffector_h

#include <Arduino.h>
#include "Curve.h"
#include "Time.h"
#include "../BottangoArduinoCallbacks.h"

#define MAX_NUM_CURVES 3

class AbstractEffector
{
public:
    AbstractEffector(int minSignal, int maxSignal);

    virtual void stop();

    bool respondsToIdentifier(char *identifier);

    virtual void getIdentifier(char *outArray, short arraySize) = 0;

    virtual void setSync(int syncValue);

    virtual void addCurve(Curve *curve);

    virtual void clearCurves();

    virtual void updateSignalBounds(int minSignal, int maxSignal, int signalSpeed);

    virtual void updateOnLoop();

    virtual void driveOnLoop();

    virtual bool useFloatCurve();

    virtual void dump();

    virtual void destroy();

    virtual ~AbstractEffector();

protected:
    int lerpSignal(float movement);

    int minSignal = 0;

    int maxSignal = 0;

    Curve *curves[MAX_NUM_CURVES]{};

    byte curvesIdx = 0;
};

#endif