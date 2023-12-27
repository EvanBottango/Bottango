#ifndef TriggerCustomEvent_h
#define TriggerCustomEvent_h

#include "AbstractEffector.h"

class TriggerCustomEvent : public AbstractEffector
{
public:
    TriggerCustomEvent(char *identifier, byte pin);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void dump() override;

protected:
private:
    char myIdentifier[9];
    bool shouldFire = false;
    byte pin = 255;
    bool pinOn = false;
    unsigned long disablePinTime = 0;
};

#endif