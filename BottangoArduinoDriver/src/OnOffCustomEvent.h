#ifndef OnOffCustomEvent_h
#define OnOffCustomEvent_h

#include "AbstractEffector.h"

class OnOffCustomEvent : public AbstractEffector
{
public:
    OnOffCustomEvent(char *identifier, bool startOn, byte pin);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void dump() override;

protected:
private:
    char myIdentifier[9];
    int currentOn = 0;
    int targetOn = 0;
    byte pin = 255;
};

#endif