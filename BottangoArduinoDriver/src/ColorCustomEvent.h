#ifndef ColorCustomEvent_h
#define ColorCustomEvent_h

#include "ColorEffector.h"
#include "Arduino.h"
#include "Color.h"

class ColorCustomEvent : public ColorEffector
{
public:
    ColorCustomEvent(char *identifier, byte startingRed, byte startingGreen, byte startingBlue);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void dump() override;

protected:
private:
    char myIdentifier[9];
};

#endif