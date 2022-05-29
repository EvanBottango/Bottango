#ifndef InterruptDrivenEffector_h
#define InterruptDrivenEffector_h

#include "AbstractEffector.h"
#include "Arduino.h"
#include "BezierCurve.h"

// for interval determining rates
#define INTERRUPT_RATE 10000
#define SIGNAL_INTERVAL_IN_TICKS 200 // 1 second is 10,000 ticks. 1/50thth of a second is (10,000 / 50) = 200 ticks
#define SIGNAL_INTERVAL_IN_MS 20     // ticks / 10

class InterruptDrivenEffector : public AbstractEffector
{
public:
    InterruptDrivenEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal);

    virtual void setSync(int syncValue, bool isTracked) override;

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;
    virtual void interruptTick() override;

    virtual void destroy() override;

    virtual void clearCurves() override;

    // public signals to set rate from
    volatile int intervalEndSignal = 0;
    volatile int nextIntervalStartSignal = 0;
    volatile unsigned long intervalStartTime = 0;
    volatile byte finalIntervalReducedTimeInMs = 255;
    volatile int syncSignal = 0;
    volatile bool readyForNext = false;

protected:
    void fillNewCurveStartingIntervals(int endSignal, unsigned long signalStartTime, byte intervalReducedTime);
    void setNextIntervalSignal(int signalOnDeck, byte intervalReducedTime);

    byte minTickRate = 0;

    // for rate tracking
    volatile byte currentTick = 0;
    volatile byte signalTickRate = 0;
    volatile byte nextTickToDrive = 0;
    volatile byte targetTickSpacingDelay = 0;
    volatile byte currenttickSpacingDelay = 0;

    volatile bool endThisInterval = false;
    volatile bool moveForward = false;
    volatile bool syncIsTracked = false;
    volatile bool waitingForNext = false;

    volatile int currentSignal = 0;

    BezierCurve *curveInProgress;
};

#endif