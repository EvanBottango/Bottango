#pragma once

#include <Arduino.h>
#include "VelocityEffector.h"

class PinStepperEffector : public VelocityEffector
{
public:
	PinStepperEffector(byte pin0, byte pin1, byte pin2, byte pin3, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxStepsPerSec, int startingStepsOffset);
	virtual void driveOnLoop() override;
	virtual void getIdentifier(char* outArray, short arraySize) override;

protected:
	byte pin0 = 0;
	byte pin1 = 0;
	byte pin2 = 0;
	byte pin3 = 0;

#ifdef PIN_REMAPPING
	byte originalPin0 = 0;
#endif

	volatile byte stepLoop = 0;

private:
	void pulse();
};