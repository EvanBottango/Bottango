#include "CurvedCustomEvent.h"
#include "../BottangoArduinoConfig.h"
#include "Errors.h"

// Signal is 0 - sig max, and just use that for movement calculations, so that this can act like a bog standard loop driven effector
CurvedCustomEvent::CurvedCustomEvent(char *identifier, float maxMovementPerSec, float startingMovement, byte pin) : LoopDrivenEffector(0, COMPRESSED_SIGNAL_MAX_INT, maxMovementPerSec * COMPRESSED_SIGNAL_MAX_INT, startingMovement * COMPRESSED_SIGNAL_MAX_INT), pin(pin)
{
    strcpy(myIdentifier, identifier);
    if (pin != 255)
    {

#ifdef NAMED_BOARD
        if (pin == 0)
        {
            Error::reportError_InvalidPin();
            return;
        }
#endif

#ifdef PIN_REMAPPING
        for (int i = 0; i < PIN_REMAP_LENGTH; i++)
        {
            if (inputPins[i] == pin)
            {
                pin = onboardPins[i];
                this->pin = pin;
                break;
            }
        }
#endif
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
        analogWrite(pin, round(startingMovement * 255));
    }
    Callbacks::onEffectorRegistered(this);
}

void CurvedCustomEvent::driveOnLoop()
{
    bool didChange = false;
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
        didChange = true;
    }
    LoopDrivenEffector::driveOnLoop();
    AbstractEffector::callbackOnDriveComplete(currentSignal, didChange);
}

void CurvedCustomEvent::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}