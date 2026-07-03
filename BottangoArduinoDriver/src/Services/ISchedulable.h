#pragma once

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
 * @brief Interface for services that want respond to different phases of the main loop.
 * 
 * Use this interface to create services that can hook into various phases of the main loop.
 */
struct ISchedulable
{
	/**
	 * @brief Virtual initialization method that performs no action in the base implementation. Override in derived classes to perform initialization.
	 * Is called once during system setup.
	 */
	virtual void init()
	{
	}

	/**
	 * @brief Virtual callback invoked when a phase occurs. Override in derived classes to handle given phase.
	 * @param p The phase being executed.
	 */
	virtual void onPhase(Phase const p)
	{
	}
};