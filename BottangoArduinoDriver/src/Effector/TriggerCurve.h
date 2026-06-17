#pragma once

#include "Curve.h"
#include "../../BottangoArduinoModules.h"

class TriggerCurve : public Curve
{
public:
#ifdef AUDIO_SD_I2S
    TriggerCurve(unsigned long startTimeInMs, unsigned long offset);
    unsigned long offset;
#else
    TriggerCurve(unsigned long startTimeInMs);
#endif

    unsigned long startTimeInMs = 0;
    bool consumed = false;
    virtual unsigned long getStartTimeMs();
};