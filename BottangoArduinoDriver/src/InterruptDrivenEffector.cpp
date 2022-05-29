#include "InterruptDrivenEffector.h"

InterruptDrivenEffector::InterruptDrivenEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal) : AbstractEffector(minSignal, maxSignal)
{
    drive = false;

    // sanity check speed
    if (maxSignalPerSec <= 0 || maxSignalPerSec > INTERRUPT_RATE)
    {
        // TODO error out
        return;
    }
    // interrupts 10000 times a second
    minTickRate = INTERRUPT_RATE / maxSignalPerSec; // how many ticks must pass before can step again
    currentSignal = startingSignal;

    curveInProgress = NULL;
}

void InterruptDrivenEffector::setSync(int syncValue, bool isTracked)
{
    drive = false;

    currentTick = 0;
    syncSignal = syncValue;
    nextTickToDrive = isTracked ? minTickRate : minTickRate * 2;
    syncIsTracked = isTracked;

    drive = true;
}

void InterruptDrivenEffector::fillNewCurveStartingIntervals(int endSignal, unsigned long curveStartTime, byte intervalReducedTime)
{
    drive = false;

    syncSignal = 0; // clear any outstanding sync if exists.

    intervalEndSignal = endSignal;
    intervalStartTime = curveStartTime;

    currentTick = 0;
    waitingForNext = false;

    finalIntervalReducedTimeInMs = intervalReducedTime;
    if (intervalReducedTime == 255)
    {
        readyForNext = true;
        endThisInterval = false;
    }
    else
    {
        readyForNext = false;
        endThisInterval = true;
        drive = true;
    }
}

void InterruptDrivenEffector::setNextIntervalSignal(int signalOnDeck, byte intervalReducedTime)
{
    nextIntervalStartSignal = signalOnDeck;
    finalIntervalReducedTimeInMs = intervalReducedTime;
    readyForNext = false;
    drive = true;
}

void InterruptDrivenEffector::updateOnLoop()
{
    bool newCurve = false;

    // need to see if a new curve could start
    if (curveInProgress == NULL)
    {
        for (int i = 0; i < MAX_NUM_CURVES; i++)
        {
            BezierCurve *curve = (BezierCurve *)curves[i];
            if (curve == NULL)
            {
                continue;
            }

            // find the curve that
            // A starts before already found
            // B starts after or at current time
            // and trash any exiting unstarted curves that start before the selected one
            if (Time::getCurrentTimeInMs() >= curve->getStartTimeMs())
            {
                if (curveInProgress != NULL)
                {
                    // prev found curve starts before this one, so trash it
                    if (curveInProgress->getStartTimeMs() < curve->getStartTimeMs())
                    {
                        free(curveInProgress);
                        curveInProgress = NULL;
                    }
                    // prev found curve starts after this one, so trash this one
                    else
                    {
                        free(curve);
                        curves[i] = NULL;
                        continue;
                    }
                }

                curveInProgress = curve;
                curves[i] = NULL;
                newCurve = true;
            }
        }
    }

    // nothing in progress, so return out after driving on loop
    if (curveInProgress == NULL)
    {
        return;
    }

    // is this a new curve
    if (newCurve)
    {
        // is the starting interval the last interval?
        byte shortened = 255;

        unsigned long intervalStartTime = curveInProgress->getStartTimeMs();
        unsigned long endSignalTime = intervalStartTime + SIGNAL_INTERVAL_IN_MS;

        // end is this first interval
        int endingSignal = 0;
        if (endSignalTime >= curveInProgress->getEndTimeMs())
        {
            endingSignal = lerpSignal(curveInProgress->getEndMovement());
            shortened = curveInProgress->getEndTimeMs() - intervalStartTime;

            // we're done with this curve, so trash it
            free(curveInProgress);
            curveInProgress = NULL;
        }
        else
        {
            endingSignal = lerpSignal(curveInProgress->getValue(endSignalTime));
        }

        // start a new curve on the effector
        fillNewCurveStartingIntervals(endingSignal, intervalStartTime, shortened);
    }

    // fill the on deck signal and shortening if effector is ready for it
    if (curveInProgress != NULL && readyForNext)
    {
        unsigned long nextIntervalTime = intervalStartTime + (SIGNAL_INTERVAL_IN_MS * 2);
        byte shorted = 255;
        int nextIntervalSignal = 0;

        // if next interval would be final interval
        if (nextIntervalTime >= curveInProgress->getEndTimeMs())
        {
            nextIntervalSignal = lerpSignal(curveInProgress->getEndMovement());
            shorted = curveInProgress->getEndTimeMs() - (intervalStartTime + SIGNAL_INTERVAL_IN_MS);

            // we're done with this curve, so trash it
            free(curveInProgress);
            curveInProgress = NULL;
        }
        // otherwise evaluate for it
        else
        {
            nextIntervalSignal = lerpSignal(curveInProgress->getValue(nextIntervalTime));
        }

        // fill the signal on the effector
        setNextIntervalSignal(nextIntervalSignal, shorted);
    }
}

void InterruptDrivenEffector::driveOnLoop()
{
    Callbacks::effectorSignalOnLoop(this, currentSignal);
}

void InterruptDrivenEffector::interruptTick()
{
    if (!drive)
    {
        return;
    }

    if (syncSignal != 0)
    {
        if (currentTick == nextTickToDrive)
        {
            driveOnInterrupt(syncSignal > 0);

            if (syncIsTracked)
            {
                currentSignal += syncSignal > 0 ? 1 : -1;
            }
            syncSignal += syncSignal > 0 ? -1 : 1;

            if (syncSignal == 0)
            {
                drive = false;
            }
            else
            {
                currentTick = 0;
            }
        }
        else
        {
            currentTick++;
        }
        return;
    }

    // 0 means the interval just started so
    // figure out the rate for this interval
    if (currentTick == 0)
    {
        // We should have had the next signal by now, but loop is running behind... so just do another tick at the same rate.
        if (readyForNext)
        {
            // only do a waiting move if it wouldn't move past min/max signal
            if ((moveForward && currentSignal + 1 > maxSignal) || (!moveForward && currentSignal - 1 < minSignal))
            {
                // bad news!
            }
            else
            {
                waitingForNext = true;
                nextTickToDrive = signalTickRate - 1; // since 0 based
            }
        }
        else
        {
            waitingForNext = false;
            // find the interval duration (either default 200, or shortened in MS)
            byte intervalDurationInTicks = SIGNAL_INTERVAL_IN_TICKS;
            if (finalIntervalReducedTimeInMs != 255)
            {
                intervalDurationInTicks = finalIntervalReducedTimeInMs * 10; // convert to Ticks
                if (intervalDurationInTicks == 0)
                {
                    intervalDurationInTicks = 1;
                }
                endThisInterval = true;
            }

            // set rate to not advance at all if already there
            if (currentSignal == intervalEndSignal)
            {
                nextTickToDrive = intervalDurationInTicks + 1; // will never get reached
            }
            // else find the rate by delta / interval
            else
            {
                // rate is delta from current to next (current so we can track error) over interval duration
                int signalDelta = abs(intervalEndSignal - currentSignal);
                signalTickRate = intervalDurationInTicks / signalDelta;
                if (signalTickRate < minTickRate)
                {
                    signalTickRate = minTickRate; // deal with max speed
                }

                // MKBUF
                // PRINT("cS: ");
                // PRINT_INT(currentSignal);
                // PRINT(" endS: ");
                // PRINT_INT(intervalEndSignal);
                // PRINT(" iT: ");
                // PRINT_INT(intervalDurationInTicks);
                // PRINT(" iR: ")
                // PRINT_INT(signalTickRate);
                // PRINT_NEWLINE();

                nextTickToDrive = signalTickRate - 1; // since 0 based

                // direction to move
                moveForward = currentSignal < intervalEndSignal;

                // deal with ideal rate falling between ticks
                // ex 80 signal over 200 ticks
                // tick rate = 2 (200 / 80 == 2.5, floored to 2)
                // finish at tick 160
                // distribute extra 40 over 200
                currenttickSpacingDelay = 0;
                targetTickSpacingDelay = 0;
                int remainderTicksAtTarget = 200 - (signalDelta * signalTickRate);
                if (remainderTicksAtTarget > 0)
                {
                    targetTickSpacingDelay = intervalDurationInTicks / remainderTicksAtTarget;
                }
            }

            readyForNext = true;
        }
    }

    // if at next tick to drive and not already at the target signal
    if (currentTick == nextTickToDrive && currentSignal != intervalEndSignal)
    {
        // just do another tick if waiting for loop to catch up
        if (waitingForNext)
        {
            currentTick = 0;
        }
        // otherwise do a regular interval
        else
        {
            nextTickToDrive = currentTick + signalTickRate;

            // add spacing to this tick if needed
            if (targetTickSpacingDelay != 0)
            {
                currenttickSpacingDelay += signalTickRate;
                if (currenttickSpacingDelay >= targetTickSpacingDelay)
                {
                    nextTickToDrive++;
                    currenttickSpacingDelay = currenttickSpacingDelay - targetTickSpacingDelay;
                }
            }
        }
        currentSignal += moveForward ? 1 : -1;

        // quick sanity check
        if (currentSignal > maxSignal || currentSignal < minSignal)
        {
            if (currentSignal > maxSignal)
            {
                currentSignal = maxSignal;
            }
            else
            {
                currentSignal = minSignal;
            }
        }
        else
        {
            driveOnInterrupt(moveForward);
        }
    }

    // completed the curve!
    if (endThisInterval && currentTick == finalIntervalReducedTimeInMs * 10)
    {
        drive = false;
        if (currentSignal != intervalEndSignal)
        {
            setSync(intervalEndSignal - currentSignal, true);
        }
    }
    // completed the interval, so reset
    else if (currentTick == SIGNAL_INTERVAL_IN_TICKS)
    {
        intervalEndSignal = nextIntervalStartSignal;
        currentTick = 0;
        intervalStartTime += SIGNAL_INTERVAL_IN_MS;
    }
    // keep going
    else
    {
        currentTick++;
    }
}

void InterruptDrivenEffector::clearCurves()
{
    AbstractEffector::clearCurves();
    if (curveInProgress != NULL)
    {
        free(curveInProgress);
        curveInProgress = NULL;
    }
}

void InterruptDrivenEffector::destroy()
{
    drive = false;
    if (curveInProgress != NULL)
    {
        free(curveInProgress);
        curveInProgress = NULL;
    }
    AbstractEffector::destroy();
}
