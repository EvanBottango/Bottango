#ifndef _PhaseScheduler_h
#define _PhaseScheduler_h

#include <Arduino.h>
#include "LoopModule.h"

/**
 * @brief The PhaseScheduler class manages the phase-based execution loop for all registered modules.
 * @details It maintains execution order based on priority and provides APIs for both core and user modules.
 *          Core modules are registered during system setup, user modules can be added via public API.
 */
class PhaseScheduler
{
public:
	/**
	 * @brief Default constructor for the PhaseScheduler class.
	 */
	PhaseScheduler() = default;

	/**
	 * @brief Default destructor for the PhaseScheduler class.
	 */
	~PhaseScheduler() = default;

	// set up the ordered list of modules
	void buildModules();

	/**
	 * @brief Initializes all registered modules (core + user) by calling their init() method.
	 * @details Called once during system startup, after setupCorePhases().
	 */
	void initModules() const;

	/**
	 * @brief Executes the specified phase on all registered modules in priority order.
	 * @param p The phase to execute.
	 */
	void executePhase(Phase p) const;

private:
	// === Combined Execution Order ===
	LoopModule *const *_orderedModules = nullptr;
	uint8_t moduleCount = 0;
};

#endif // _PhaseScheduler_h
