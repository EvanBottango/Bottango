#ifndef _PhaseScheduler_h
#define _PhaseScheduler_h

#include <Arduino.h>
#include "LoopModule.h"

// Maximum number of core modules (system-defined)
#ifndef MAX_CORE_MODULES
#define MAX_CORE_MODULES 16
#endif

// Maximum number of user modules (can be overridden in BottangoArduinoModules.h)
#ifndef MAX_USER_MODULES
#define MAX_USER_MODULES 8
#endif

/**
 * @brief Internal structure to track modules with their priority.
 */
struct ModuleEntry
{
	LoopModule* module;
	Priority priority;
};

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

	/**
	 * @brief Sets up core modules by retrieving them from the factory and registering them in priority order.
	 * @details Called once during system startup, after factory setup and wiring.
	 */
	void setupCorePhases();

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

	/**
	 * @brief Registers a user-defined module to participate in the execution loop.
	 * @param userModule Pointer to the user module (must remain valid for the lifetime of the program).
	 * @param priority Priority level for execution order (default: Normal).
	 * @details The module's init() method is called immediately after registration.
	 */
	void addToLoop(LoopModule* userModule, Priority priority = Priority::Normal);

private:
	/**
	 * @brief Registers a core module with a specific priority.
	 * @param module Pointer to the module.
	 * @param priority Priority level for execution order.
	 */
	void registerCoreModule(LoopModule* module, Priority priority);

	/**
	 * @brief Sorts all modules by priority (ascending order).
	 * @details Called after all core modules are registered and whenever a user module is added.
	 */
	void sortModulesByPriority();

	// === Core Modules ===
	ModuleEntry _coreModules[MAX_CORE_MODULES];
	uint8_t _coreModuleCount = 0;

	// === User Modules ===
	ModuleEntry _userModules[MAX_USER_MODULES];
	uint8_t _userModuleCount = 0;

	// === Combined Execution Order ===
	// After sorting, this array contains all modules (core + user) in priority order
	LoopModule* _executionOrder[MAX_CORE_MODULES + MAX_USER_MODULES];
	uint8_t _executionCount = 0;
};

#endif // _PhaseScheduler_h