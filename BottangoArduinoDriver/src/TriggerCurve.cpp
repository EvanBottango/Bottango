#include "TriggerCurve.h"

#ifdef AUDIO_SD_I2S
TriggerCurve::TriggerCurve(unsigned long startTimeInMs, unsigned long offset) : Curve()
{
    this->offset = offset;
#else
TriggerCurve::TriggerCurve(unsigned long startTimeInMs) : Curve()
{
#endif
    this->startTimeInMs = startTimeInMs;
    consumed = false;
};