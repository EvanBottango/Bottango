#ifndef TriggerCustomEvent_h
#define TriggerCustomEvent_h

#include "AbstractEffector.h"

class TriggerCustomEvent : public AbstractEffector
{
public:
    TriggerCustomEvent(char *identifier, byte pin, bool fireIsHigh);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;

protected:
private:
    char myIdentifier[9];
    bool shouldFire = false;
    byte pin = 255;
    bool pinOn = false;
    bool fireIsHigh = true;
    unsigned long disablePinTime = 0;
};

#endif