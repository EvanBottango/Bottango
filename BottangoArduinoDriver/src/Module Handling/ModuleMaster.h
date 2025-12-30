// ModuleMaster.h

#ifndef _ModuleMaster_h
#define _ModuleMaster_h

//#define MAX_MODULES	10

#include <Arduino.h>
#include "ModuleLoop.h"

/**
 * @brief Enumeration of available modules.
 */
 // ToDo: There are two ways of doing this. We can use this enum class, or we can use a registration system where modules register themselves.
 // The second approach would need a Map<> to have a way to look up modules by name or type.
 // First approach is simple and straight forward, but requires updating this enum class whenever a new module is added.
 // Second approach is more flexible and extensible, but adds complexity.
enum class Modules : uint8_t
{
	StopButton,
	StatusLights,
	AudioI2S,
	Max
};

class ModuleMaster
{
public:


	ModuleMaster();

	/**
	 * @brief Initializes and configures modules used by the application.
	 */
	void setupModules();

	/**
	 * @brief Initializes modules. Is called once during system setup.
	 */
	void initModules();

	/**
	 * @brief Executes the specified phase. Loops through all registered modules and calls their onPhase method.
	 * @param p The phase to execute.
	 */
	void executePhase(Phase p);

private:
	/**
	 * @brief An array of pointers to registered LoopModule instances.
	 */
	LoopModule* modules[(int)Modules::Max];
};

class InterfaceRegistry
{
public:
	/**
	 * @brief Registers the interface for the given module type.
	 * @param ModuleType The type of module.
	 * @param interface Pointer to the interface to register.
	 */
	static void registerInterface(Modules ModuleType, void* interface)
	{
		interfaces[(int)ModuleType] = interface;
	}

	/**
	 * @brief Retrieves the interface for the given module type.
	 * @param ModuleType The type of module.
	 * @return Pointer to the registered interface.
	 */
	static void* get(Modules ModuleType)
	{
		return interfaces[(int)ModuleType];
	}

private:
	/**
	 * @brief A static inline array of void* pointers named interfaces, sized to (int)Modules::Max and initialized with nullptr.
	 */
	static inline void* interfaces[(int)Modules::Max] = { nullptr };
};

#endif