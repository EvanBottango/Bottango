
#ifndef BOTTANGOARDUINO_TRIGGERCURVE_H
#define BOTTANGOARDUINO_TRIGGERCURVE_H

#include "Curve.h"
#include "../BottangoArduinoModules.h"

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
};

#endif // BOTTANGOARDUINO_TRIGGERCURVE_H