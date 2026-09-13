#pragma once

#include <Arduino.h>

/**
 * @brief Interface for all services, such as data sources, that can be placed in a service slot.
 * 
 * See ServiceSlot.h, specifically the ServiceSlot template class, for more information on what this interface is used for.
 * Services that implement this interface can be placed in a ServiceSlot, which allows for dynamic swapping of service implementations at runtime.
 */
class IService
{
	IService()
	{
	}

public:
	virtual ~IService() = default;

	/**
	 * @brief Is automatically called when the service is activated / placed into a service slot
	 */
	virtual void onActivation() = 0;
};