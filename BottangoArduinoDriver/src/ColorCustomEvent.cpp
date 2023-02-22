#include "ColorCustomEvent.h"
#include "Log.h"

// Signal is 0 - 255, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
ColorCustomEvent::ColorCustomEvent(char *identifier, byte startingRed, byte startingGreen, byte startingBlue) : ColorEffector(startingRed, startingGreen, startingBlue)
{
    strcpy(myIdentifier, identifier);

    Callbacks::onEffectorRegistered(this);
}

void ColorCustomEvent::driveOnLoop()
{
    if (currentColor.r != targetColor.r || currentColor.g != targetColor.g || currentColor.b != targetColor.b)
    {
        currentColor.r = targetColor.r;
        currentColor.g = targetColor.g;
        currentColor.b = targetColor.b;

        Callbacks::onColorCustomEventColorChanged(this, currentColor.r, currentColor.g, currentColor.b);
    }
    ColorEffector::driveOnLoop();
}

void ColorCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void ColorCustomEvent::dump()
{
    LOG_LN(F("= Color Event DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}