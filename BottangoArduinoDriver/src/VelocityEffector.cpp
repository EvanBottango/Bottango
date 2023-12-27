#include "VelocityEffector.h"
#include "Log.h"
#include "Outgoing.h"

VelocityEffector::VelocityEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal) : AbstractEffector(minSignal, maxSignal)
{
    if (maxSignalPerSec <= 0)
    {
        // TODO ERROR
        return;
    }
    this->minMicrosPerSignal = 1000000L / maxSignalPerSec;

    this->currentSignal = startingSignal;
    this->targetSignal = startingSignal;
    this->drive = 0;
    this->lastSignalChangeTimeUs = 0;
    this->signalChangePeriodUs = 0;
    this->inProgressCurveIdx = 0;
}

void VelocityEffector::setSync(int syncValue)
{
    sync = syncValue;
    signalChangePeriodUs = minMicrosPerSignal * STEPPER_SYNC_SPEED; // move at half max speed
    lastSignalChangeTimeUs = micros();
}

void VelocityEffector::endAutoSync()
{
    sync = 0;
    Outgoing::outgoing_notifySyncComplete();
    char effectorIdentifier[9];
    getIdentifier(effectorIdentifier, 9);
    Serial.write(effectorIdentifier);
    Serial.write("\n");
}

void VelocityEffector::updateSignalBounds(int minSignal, int maxSignal, int signalSpeed)
{
    AbstractEffector::updateSignalBounds(minSignal, maxSignal, signalSpeed);
    this->minMicrosPerSignal = 1000000L / signalSpeed;
}

void VelocityEffector::updateOnLoop()
{
    // overall flow:
    // find first curve in progress (should be only)
    //      if none found, go to end of last in progress curve
    // else
    //      find the target singal one click worth into the curve
    //      if target signal == current signal, sleep one click worth
    //      otherwise move towards the target signal over 1 click length, at the rate of change to arrive at the correct signal when time is up.

    // an outstanding signal change, so just sleep
    if (drive != 0)
    {
        return;
    }

    // in in progress curve had no change at next click time
    // so we sleep this update till the next click start
    if (sleepStartTime > 0)
    {
        if (millis() - sleepStartTime >= VELOCITY_SEGMENT_MS)
        {
            // exit sleep
            sleepStartTime = 0;
        }
        else
        {
            // wait for sleep to finish
            return;
        }
    }

    unsigned long currentTimeMs = Time::getCurrentTimeInMs();
    unsigned long nowUS = micros();

    // set drive value if we're evaluating a loop
    // an outstanding signal change, so just sleep

    // deal with sync
    if (sync != 0)
    {
        if (nowUS - lastSignalChangeTimeUs >= signalChangePeriodUs)
        {
            drive = sync > 0 ? -1 : 1;
            lastSignalChangeTimeUs = nowUS;
        }
        return;
    }

    // are in velocity progress?
    // signalChangePeriod must be > 0 in order to be working, cannot move faster than 1 signal per uS
    if (currentSignal != targetSignal && signalChangePeriodUs > 0)
    {
        // at the next update time?
        if (nowUS - lastSignalChangeTimeUs >= signalChangePeriodUs)
        {
            // curve still valid
            BezierCurve *velocityInProgressCurve = NULL;

            // in progress curve Idx will be 255 if we're just moving to the end of the last curve.

            // find the curve in progress if not moving to end of last
            if (inProgressCurveIdx != 255)
            {
                velocityInProgressCurve = (BezierCurve *)curves[inProgressCurveIdx];
            }

            // kill this velocity progress if...
            // curve is not found
            // curve is finished
            if (inProgressCurveIdx != 255 && (velocityInProgressCurve == NULL || velocityInProgressCurve->getEndTimeMs() < currentTimeMs))
            {
                // reset if not
                targetSignal = currentSignal;
                signalChangePeriodUs = 0;
                // and continue in method from here...
            }
            // time to drive
            else
            {
                // set drive, and next signal change time
                drive = currentSignal < targetSignal ? 1 : -1;
                lastSignalChangeTimeUs = nowUS;

                // at end of click?
                if (currentSignal + drive == targetSignal)
                {
                    // exit this click
                    signalChangePeriodUs = 0;
                    inProgressCurveIdx = 0;
                }

                // repeat drive if period abort time is 0 or not met
                // otherwise abort out of drive and get a new velocity target
                if (periodAbortTime == 0 || currentTimeMs < periodAbortTime)
                {
                    return;
                }
            }
        }
        else
        {
            return; // wait for signal change time
        }
    }
    else
    {
        // reset otherwise and
        targetSignal = currentSignal;
        signalChangePeriodUs = 0;
        // will continue in method from here...
    }

    // continue on and find the next velocity click
    BezierCurve *lastCurve = NULL;
    unsigned long nextClickTime = currentTimeMs + VELOCITY_SEGMENT_MS;

    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        BezierCurve *curve = (BezierCurve *)curves[i];
        if (curve == NULL)
        {
            continue;
        }

        // found a curve in progress
        if (curve->isInProgress(nextClickTime))
        {
            // get rate limited signal at next click time
            // IE now + click length
            float movement = curve->getValue(nextClickTime);
            targetSignal = lerpSignal(movement);

            // how much signal to move
            int signalDelta = abs(currentSignal - targetSignal);

            if (signalDelta > 0)
            {
                // over how long in us?
                unsigned long timeDeltaUs = VELOCITY_SEGMENT_MS * 1000L;

                // ex move 50 signal
                // over 100,000 us (IE 1/10th a second)
                // means move one signal every 2000 us
                signalChangePeriodUs = timeDeltaUs / signalDelta;
                if (signalChangePeriodUs < minMicrosPerSignal)
                {
                    signalChangePeriodUs = minMicrosPerSignal;
                }

                inProgressCurveIdx = i;

                // are we currently in an overshoot, and need to break out of evaluation from a period before hitting target?
                // this will happen if you animate faster than the set max speeed
                // unfortunatley when you happen to already be moving at max speed, there's a slight hitch with the calculation, so this tries to only
                // do this abort out when it's anticipated to be needed.
                int curveEndSignal = lerpSignal(curve->getEndMovement());
                if ((currentSignal < targetSignal && currentSignal > curveEndSignal) || (currentSignal > targetSignal && currentSignal < curveEndSignal))
                {
                    periodAbortTime = nextClickTime + VELOCITY_SEGMENT_MS;
                }
                else
                {
                    periodAbortTime = 0;
                }
            }
            else
            {
                sleepStartTime = millis();
            }
            return;
        }
        // set as last curve if is last curve
        else
        {
            if (lastCurve == NULL || curve->getEndTimeMs() > lastCurve->getEndTimeMs())
            {
                lastCurve = curve;
            }
        }
    }

    // If no curves were in progress, go to the final known state of last curve, if any
    if (lastCurve != NULL)
    {
        if (lastCurve->getEndTimeMs() < currentTimeMs)
        {
            int endTarget = lerpSignal(lastCurve->getEndMovement());

            if (targetSignal != endTarget)
            {
                targetSignal = endTarget;

                int signalDelta = abs(currentSignal - targetSignal);

                if (signalDelta >= 0)
                {
                    signalChangePeriodUs = minMicrosPerSignal;
                    inProgressCurveIdx = 255;
                }
            }
        }
    }
}

void VelocityEffector::driveOnLoop()
{
    Callbacks::effectorSignalOnLoop(this, currentSignal);
    AbstractEffector::driveOnLoop();
}

bool VelocityEffector::useFloatCurve()
{
#ifdef __AVR__

#ifdef AVR_STEPPER_FIXED
    return false;
#else
    return AbstractEffector::useFloatCurve();
#endif

#else
    return AbstractEffector::useFloatCurve();
#endif
}