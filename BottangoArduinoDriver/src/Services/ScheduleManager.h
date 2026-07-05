#pragma once

#include <Arduino.h>
#include "ISchedulable.h"

/**
 * @brief The ScheduleManager class manages the phase-based execution loop for all registered services.
 * @details It maintains execution order based on hard coded registration order.
 */
class ScheduleManager
{
public:
	/**
	 * @brief Default constructor for the ScheduleManager class.
	 */
	ScheduleManager() = default;

	/**
	 * @brief Default destructor for the ScheduleManager class.
	 */
	~ScheduleManager() = default;

	/**
	 * @brief Creates a static array and populates it with all enabled services. The order in this array is the call order for these services during the main loop.
	 */
	void buildServices();

	/**
	 * @brief Initializes all registered services by calling their init() method.
	 * @details Called once during system startup, after buildServices().
	 */
	void initServices() const;

	/**
	 * @brief Executes the specified phase on all registered services. The order of execution is determined by the order in which services were registered.
	 * @param p The phase to execute.
	 */
	void executePhase(Phase const p) const;

private:
	/**
	 * @brief Contains all registered services in the order they should be executed during the main loop.
	 */
	ISchedulable* const* m_orderedServices = nullptr;
	
	uint8_t m_servicesCount = 0;
};