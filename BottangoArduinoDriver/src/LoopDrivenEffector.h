#ifndef LoopDrivenEffector_h
#define LoopDrivenEffector_h

#include "AbstractEffector.h"
#include "Arduino.h"
#include "Time.h"
#include "BezierCurve.h"

class LoopDrivenEffector : public AbstractEffector
{
public:
    LoopDrivenEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

protected:
    unsigned long minMicrosPerSignal = 0;

    int targetSignal = 0;
    int currentSignal = 0;

    unsigned long lastUpdateTimeInUS = 0;

    int speedLimitSingal(int newTarget, unsigned long nowInUS);
};

#endif