#ifndef VelocityEffector_h
#define VelocityEffector_h

#include "AbstractEffector.h"
#include "../BottangoArduinoConfig.h"
#include "BasicCommands.h"
#include "Arduino.h"
#include "Time.h"
#include "BezierCurve.h"
#include "FixedBezierCurve.h"

class VelocityEffector : public AbstractEffector
{
public:
    VelocityEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal);
    virtual void updateSignalBounds(int minSignal, int maxSignal, int signalSpeed) override;
    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;
    virtual void setSync(int syncValue) override;
    virtual bool useFloatCurve() override;

protected:
    // for speed limiting
    unsigned long minMicrosPerSignal = 0;

    // for driving
    int targetSignal = 0;
    int currentSignal = 0;
    int sync = 0;
    int drive = 0;
    unsigned int signalChangePeriodUs = 0;

    // for timing
    unsigned long lastSignalChangeTimeUs = 0;
    unsigned long sleepStartTime = 0;
    unsigned long periodAbortTime = 0;

    byte inProgressCurveIdx;

    bool updateDrive(unsigned long nowUS, unsigned long currentTimeMs);
    void endAutoSync();
};

#endif