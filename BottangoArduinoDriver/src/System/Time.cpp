#include "Time.h"

namespace Time
{
	unsigned long lastSyncdTime = 0;
	long timeOffset = 0;
#ifdef RELAY_SUPPORTED
	bool timeStopped = false;
#endif

	void syncTime(unsigned long incomingTime)
	{
#ifdef RELAY_SUPPORTED
		timeStopped = false;
#endif
		lastSyncdTime = incomingTime;
		timeOffset = incomingTime - millis();
	}

	unsigned long getCurrentTimeInMs()
	{
#ifdef RELAY_SUPPORTED
		if (timeStopped)
		{
			return 0;
		}
#endif
		return millis() + timeOffset;
	}

	unsigned long getLastSyncedTimeInMs()
	{
		return lastSyncdTime;
	}

#ifdef RELAY_SUPPORTED
	void stopTime()
	{
		lastSyncdTime = 0;
		timeOffset = 0;
		timeStopped = true;
	}
#endif

} // namespace  Time