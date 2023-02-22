#include "CurvedCustomEvent.h"
#include "Log.h"
#include "../BottangoArduinoConfig.h"

// Signal is 0 - sig max, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
CurvedCustomEvent::CurvedCustomEvent(char *identifier, float maxMovementPerSec, float startingMovement) : LoopDrivenEffector(0, COMPRESSED_SIGNAL_MAX_INT, maxMovementPerSec * COMPRESSED_SIGNAL_MAX_INT, startingMovement * COMPRESSED_SIGNAL_MAX_INT)
{
    strcpy(myIdentifier, identifier);

    Callbacks::onEffectorRegistered(this);
}

void CurvedCustomEvent::driveOnLoop()
{
    if (currentSignal != targetSignal)
    {
        // callback here
        currentSignal = targetSignal;
        Callbacks::onCurvedCustomEventMovementChanged(this, (float)currentSignal * 0.001f);
    }
    LoopDrivenEffector::driveOnLoop();
}

void CurvedCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void CurvedCustomEvent::dump()
{
    LOG_LN(F("= Curved Event DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}