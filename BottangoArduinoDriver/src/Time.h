#ifndef Time_h
#define Time_h

#include "Arduino.h"

namespace Time
{
    void syncTime(unsigned long incomingTime);

    unsigned long getCurrentTimeInMs();

    unsigned long getLastSyncedTimeInMs();

} // namespace Time
#endif