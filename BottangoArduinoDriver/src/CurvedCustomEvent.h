#ifndef CurvedCustomEvent_h
#define CurvedCustomEvent_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"

class CurvedCustomEvent : public LoopDrivenEffector
{
public:
    CurvedCustomEvent(char *identifier, float maxMovementPerSec, float startingMovement, byte pin);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void dump() override;

protected:
private:
    char myIdentifier[9];
    byte pin = 255;
};

#endif