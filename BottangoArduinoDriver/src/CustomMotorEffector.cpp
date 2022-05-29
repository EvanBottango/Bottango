#include "CustomMotorEffector.h"
#include "Log.h"

CustomMotorEffector::CustomMotorEffector(char *identifier, short minSignal, short maxSignal, int maxSignalSec, short startSignal) : LoopDrivenEffector(minSignal, maxSignal, maxSignalSec, startSignal)
{
    strcpy(myIdentifier, identifier);
    Callbacks::onEffectorRegistered(this);
    Callbacks::effectorSignalOnLoop(this, startSignal);
}

void CustomMotorEffector::driveOnLoop()
{
    if (currentSignal != targetSignal)
    {
        currentSignal = targetSignal;
    }
    LoopDrivenEffector::driveOnLoop();
}

void CustomMotorEffector::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}

void CustomMotorEffector::dump()
{
    LOG_LN(F("= CUSTOM MOTOR DUMP ="))
    AbstractEffector::dump();
    LOG_LN(F("=="))
}