#ifndef _LoopModule_h
#define _LoopModule_h

#include <Arduino.h>

/**
 * @brief The different phases of the main loop. Modules can hook into these phases to perform actions at specific points in the loop.
 */
enum class Phase : uint8_t
{
	Early,
	Input,
	Communication,
	Logic,
	Output,
	Late
};

/**
 * @brief Priority levels for module execution order.
 * @details Lower numeric values execute first. Core modules use fixed priorities,
 *          user modules can use Normal or Late priorities.
 */
enum class Priority : uint8_t
{
	VeryEarly = 0,      // Reserved for critical core modules (DataSources)
	Early = 50,         // Reserved for core processing modules (Decoder, Parser)
	Normal = 100,       // Default priority for user modules
	Late = 150,         // For modules that should run after most other modules
	VeryLate = 200      // Reserved for cleanup/finalization modules
};

/**
 * @brief Base class for modules that can respond to different phases of the main loop.
 * 
 * Derive from this class to create modules that can hook into various phases of the main loop.
 */
class LoopModule
{
public:
	LoopModule() = default;
	virtual ~LoopModule() = default;

	/**
	 * @brief Virtual initialization method that performs no action in the base implementation. Override in derived classes to perform initialization.
	 * Is called once during system setup.
	 */
	virtual void init() {}

	/**
	 * @brief Virtual callback invoked when a phase occurs. Override in derived classes to handle given phase.
	 * @param p The phase being executed.
	 */
	virtual void onPhase(Phase const p) {}
};

#endif