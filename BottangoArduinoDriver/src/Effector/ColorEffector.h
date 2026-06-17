#pragma once

#include <Arduino.h>
#include "AbstractEffector.h"
#include "ColorCurve.h"
#include "../System/Time.h"

class ColorEffector : public AbstractEffector
{
public:
	ColorEffector(byte startingRed, byte startingGreen, byte startingBlue);

	virtual void updateOnLoop() override;
	virtual void driveOnLoop() override;

protected:
	Color currentColor;
	Color targetColor;
};