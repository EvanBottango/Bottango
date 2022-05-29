#include "LoopDrivenEffector.h"
#include "Log.h"

LoopDrivenEffector::LoopDrivenEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal) : AbstractEffector(minSignal, maxSignal)
{
    if (maxSignalPerSec <= 0)
    {
        // TODO ERROR
        return;
    }
    this->minMicrosPerSignal = 1000000L / maxSignalPerSec;

    this->currentSignal = startingSignal;
    this->targetSignal = startingSignal;

    this->lastUpdateTimeInUS = micros();
}

void LoopDrivenEffector::updateOnLoop()
{
    unsigned long currentTime = Time::getCurrentTimeInMs();

    BezierCurve *lastCurve = NULL;
    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        BezierCurve *curve = (BezierCurve *)curves[i];
        if (curve == NULL)
        {
            continue;
        }

        if (curve->isInProgress(currentTime))
        {
            // Curve values are 0-1, but we want to map to the range [minPWM-maxPWM]
            float movement = curve->getValue(currentTime);
            int newTargetSignal = lerpSignal(movement); // 1400 - 1500 // where does the curve say to be

            unsigned long nowInUS = micros();
            int rateLimitedTarget = speedLimitSingal(newTargetSignal, nowInUS);

            if (targetSignal != rateLimitedTarget)
            {
                lastUpdateTimeInUS = nowInUS;
                targetSignal = rateLimitedTarget;
            }
            return;
        }
        else
        {
            if (lastCurve == NULL || curve->getEndTimeMs() > lastCurve->getEndTimeMs())
            {
                lastCurve = curve;
            }
        }
    }

    // If no curves were in progress, go to the final known state
    if (lastCurve != NULL)
    {
        if (lastCurve->getEndTimeMs() < currentTime)
        {
            unsigned long nowInUS = micros();

            int endTarget = lerpSignal(lastCurve->getEndMovement());

            if (targetSignal != endTarget)
            {
                int rateLimitedTarget = speedLimitSingal(endTarget, nowInUS);

                if (targetSignal != rateLimitedTarget)
                {
                    targetSignal = rateLimitedTarget;
                    lastUpdateTimeInUS = nowInUS;
                }
            }
        }
    }
}

int LoopDrivenEffector::speedLimitSingal(int newTarget, unsigned long nowInUS)
{

    int returnSignal = newTarget;

    unsigned int maxSignalInElapsedTime = (nowInUS - lastUpdateTimeInUS) / minMicrosPerSignal;

    if (abs(currentSignal - returnSignal) > maxSignalInElapsedTime) // 100 > 50 // delta from
    {
        if (currentSignal < returnSignal) //move forward
        {
            returnSignal = currentSignal + maxSignalInElapsedTime;
        }
        else //move backward
        {
            returnSignal = currentSignal - maxSignalInElapsedTime;
        }
    }

    if (returnSignal > maxSignal)
    {
        returnSignal = maxSignal;
    }
    else if (returnSignal < minSignal)
    {
        returnSignal = minSignal;
    }

    return returnSignal;
}

void LoopDrivenEffector::driveOnLoop()
{
    Callbacks::effectorSignalOnLoop(this, currentSignal);
}