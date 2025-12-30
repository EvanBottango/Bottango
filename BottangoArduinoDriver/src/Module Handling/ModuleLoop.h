// ModuleLoop.h

#ifndef _ModuleLoop_h
#define _ModuleLoop_h

#include <Arduino.h>

enum class Phase
{
	Early,
	Input,
	Communication,
	Logic,
	Output,
	Late
};

class LoopModule
{
public:
	/**
	 * @brief Virtual callback invoked when a phase occurs. Override in derived classes to handle given phase.
	 * @param p The phase being executed.
	 */
	virtual void onPhase(Phase p) {}

	/**
	 * @brief Virtual initialization method that performs no action in the base implementation. Override in derived classes to perform initialization.
	 * Is called once during system setup.
	 */
	virtual void init() {}
};


#endif

