#include "Time.h"

namespace Time
{
    unsigned long lastSyncdTime = 0;
    long timeOffset = 0;

    void syncTime(unsigned long incomingTime)
    {
        lastSyncdTime = incomingTime;
        timeOffset = incomingTime - millis();
    }

    unsigned long getCurrentTimeInMs()
    {
        return millis() + timeOffset;
    }

    unsigned long getLastSyncedTimeInMs()
    {
        return lastSyncdTime;
    }

} // namespace  Time
