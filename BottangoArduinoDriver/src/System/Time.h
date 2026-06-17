#pragma once

#include "Arduino.h"
#include "../../BottangoArduinoModules.h"

namespace Time
{
	void syncTime(unsigned long incomingTime);

	unsigned long getCurrentTimeInMs();

	unsigned long getLastSyncedTimeInMs();

#ifdef RELAY_SUPPORTED
	void stopTime();
	extern bool timeStopped;
#endif

} // namespace Time