#ifndef EffectorPool_h
#define EffectorPool_h

#include "AbstractEffector.h"
#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"

class EffectorPool
{

public:
    EffectorPool();

    void addEffector(AbstractEffector *effector);

    void removeEffector(char *identifier);

    void addCurveToEffector(char *identifier, Curve *curve);

    void updateEffectorSignalBounds(char *identifier, int minSignal, int maxSignal, int signalSpeed);

    void syncEffector(char *identifier, int syncValue);

    void clearCurvesForEffector(char *identifier);

    void updateAllDriveTargets();

    void deregisterAll();

    void clearAllCurves();

    void dump();

    bool effectorUsesFloatCurve(char *identifier);

private:
    AbstractEffector *getEffector(char *identifier);

    CircularArray<AbstractEffector> effectors = CircularArray<AbstractEffector>(MAX_REGISTERED_EFFECTORS);
};

#endif // BOTTANGOARDUINO_SERVOPOOL_H
