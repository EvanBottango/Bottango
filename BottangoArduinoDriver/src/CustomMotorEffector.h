#ifndef CustomMotorEffector_h
#define CustomMotorEffector_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"

class CustomMotorEffector : public LoopDrivenEffector
{
public:
    CustomMotorEffector(char *identifier, short minSignal, short maxSignal, int maxSignalSec, short startSignal);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void dump() override;

protected:
private:
    char myIdentifier[9];
};

#endif