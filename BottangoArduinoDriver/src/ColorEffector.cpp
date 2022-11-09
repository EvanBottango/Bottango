#include "ColorEffector.h"

ColorEffector::ColorEffector(byte startingRed, byte startingGreen, byte startingBlue) : AbstractEffector(0, 1)
{
    this->currentColor.r = startingRed;
    this->currentColor.g = startingGreen;
    this->currentColor.b = startingBlue;

    this->targetColor.r = startingRed;
    this->targetColor.g = startingGreen;
    this->targetColor.b = startingBlue;
}

void ColorEffector::updateOnLoop()
{
    unsigned long currentTime = Time::getCurrentTimeInMs();

    ColorCurve *lastCurve = NULL;
    for (int i = 0; i < MAX_NUM_CURVES; ++i)
    {
        ColorCurve *curve = (ColorCurve *)curves[i];
        if (curve == NULL)
        {
            continue;
        }

        if (curve->isInProgress(currentTime))
        {
            // Curve values are 0-1, but we want to map to the range [minPWM-maxPWM]
            Color curveColor = curve->getValue(currentTime);
            targetColor.r = curveColor.r;
            targetColor.g = curveColor.g;
            targetColor.b = curveColor.b;
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
            Color endColor = lastCurve->getEndColor();
            targetColor.r = endColor.r;
            targetColor.g = endColor.g;
            targetColor.b = endColor.b;
        }
    }
}

void ColorEffector::driveOnLoop()
{
    // todo, cant work with 16 bit int
    //Callbacks::effectorSignalOnLoop(this, currentOn);
}
