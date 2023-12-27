#include "CurvedCustomEvent.h"
#include "Log.h"
#include "../BottangoArduinoConfig.h"

// Signal is 0 - sig max, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
CurvedCustomEvent::CurvedCustomEvent(char *identifier, float maxMovementPerSec, float startingMovement, byte pin) : LoopDrivenEffector(0, COMPRESSED_SIGNAL_MAX_INT, maxMovementPerSec * COMPRESSED_SIGNAL_MAX_INT, startingMovement * COMPRESSED_SIGNAL_MAX_INT), pin(pin)
{
    strcpy(myIdentifier, identifier);
    if (pin != 255)
    {
        pinMode(pin, OUTPUT);
    }
    Callbacks::onEffectorRegistered(this);
}

void CurvedCustomEvent::driveOnLoop()
{
    if (currentSignal != targetSignal)
    {
        // callback here
        currentSignal = targetSignal;
        float movement = currentSignal / COMPRESSED_SIGNAL_MAX;
        Callbacks::onCurvedCustomEventMovementChanged(this, movement);

        if (pin != 255)
        {
            analogWrite(pin, round(movement * 255));
        }
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