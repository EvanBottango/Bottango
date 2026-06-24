#pragma once

#include <Arduino.h>
#include "LoopDrivenEffector.h"

class CustomMotorEffector : public LoopDrivenEffector
{
public:
	CustomMotorEffector(char* identifier, short minSignal, short maxSignal, int maxSignalSec, short startSignal);
	virtual void driveOnLoop() override;

	virtual void getIdentifier(char* outArray, short arraySize) override;

protected:
private:
	char myIdentifier[9];
};