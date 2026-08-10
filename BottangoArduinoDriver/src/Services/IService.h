#pragma once

#include <Arduino.h>

class IService
{
	IService()
	{
	}

	~IService()
	{
	}

	/**
	 * @brief Is automatically called when the service is activated / placed into a service slot
	 */
	virtual void onActivation() = 0;
};