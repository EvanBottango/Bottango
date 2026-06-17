#pragma once

#include <Arduino.h>
#include "LoopDrivenEffector.h"

class CurvedCustomEvent : public LoopDrivenEffector
{
public:
	CurvedCustomEvent(char* identifier, float maxMovementPerSec, float startingMovement, byte pin);
	virtual void driveOnLoop() override;

	virtual void getIdentifier(char* outArray, short arraySize) override;

protected:
private:
	char myIdentifier[9];
	byte pin = 255;
};