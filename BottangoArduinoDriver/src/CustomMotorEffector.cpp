#include "CustomMotorEffector.h"
#include "Log.h"

CustomMotorEffector::CustomMotorEffector(char *identifier, short minSignal, short maxSignal, int maxSignalSec, short startSignal) : LoopDrivenEffector(minSignal, maxSignal, maxSignalSec, startSignal)
{
    strcpy(myIdentifier, identifier);
    Callbacks::onEffectorRegistered(this);
}

void CustomMotorEffector::driveOnLoop()
{
    bool didChange = false;
    if (currentSignal != targetSignal)
    {
        currentSignal = targetSignal;
        didChange = true;
    }
    LoopDrivenEffector::driveOnLoop();
    AbstractEffector::callbackOnDriveComplete(currentSignal, didChange);
}

void CustomMotorEffector::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, myIdentifier);
}