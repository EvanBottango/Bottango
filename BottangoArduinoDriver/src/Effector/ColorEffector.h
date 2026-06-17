#ifndef ColorEffector_h
#define ColorEffector_h

#include "AbstractEffector.h"
#include "Arduino.h"
#include "Time.h"
#include "ColorCurve.h"

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

#endif