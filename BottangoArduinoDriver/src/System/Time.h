#pragma once

#include "Arduino.h"
#include "../../BottangoArduinoModules.h"

namespace Time
{
	/**
	 * @brief Synchronizes the internal time with an incoming time value. This function updates the last synchronized time and calculates the time offset based on the current millis() value.
	 * @param incomingTime The time value to synchronize with, in milliseconds.
	 */
	void syncTime(unsigned long incomingTime);

	/**
	 * @brief Returns the current time in milliseconds, adjusted by the time offset. If time is stopped (in relay-supported mode), it returns 0.
	 * @return The current time in milliseconds, adjusted by the time offset.
	 */
	unsigned long getCurrentTimeInMs();

	/**
	 * @brief Returns the last time value that was passed to syncTime().
	 * @return The last incomingTime value that was passed to syncTime(), in milliseconds.
	 */
	unsigned long getLastSyncedTimeInMs();

#ifdef RELAY_SUPPORTED
	/**
	 * @brief Stops the time tracking, effectively freezing the current time. This function is only available in relay-supported mode.
	 */
	void stopTime();

	/**
	 * @brief Indicates whether the time tracking is currently stopped. This variable is only available in relay-supported mode.
	 */
	extern bool timeStopped;
#endif

} // namespace Time