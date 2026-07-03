#pragma once

#include <Arduino.h>
#include "ISchedulable.h"

/**
 * @brief The ScheduleManager class manages the phase-based execution loop for all registered modules.
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
	 * @brief Creates the orderedModules array and populates it with all enabled modules in the correct execution order.
	 */
	void buildModules();

	/**
	 * @brief Initializes all registered modules by calling their init() method.
	 * @details Called once during system startup, after buildModules().
	 */
	void initModules() const;

	/**
	 * @brief Executes the specified phase on all registered modules in priority order.
	 * @param p The phase to execute.
	 */
	void executePhase(Phase const p) const;

private:
	// === Combined Execution Order ===
	// After sorting, this array contains all modules in priority order
	ISchedulable* const* m_orderedModules = nullptr;
	uint8_t m_moduleCount = 0;
};