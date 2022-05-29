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

    void manualSyncEffector(char *identifier, int syncValue);

    void clearCurvesForEffector(char *identifier);

    void updateAllDriveTargets();

    void interruptDriveLoop();

    void deregisterAll();

    void clearAllCurves();

    void dump();

private:
    AbstractEffector *getEffector(char *identifier);

    CircularArray<AbstractEffector> effectors = CircularArray<AbstractEffector>(MAX_REGISTERED_EFFECTORS);
};

#endif //BOTTANGOARDUINO_SERVOPOOL_H
