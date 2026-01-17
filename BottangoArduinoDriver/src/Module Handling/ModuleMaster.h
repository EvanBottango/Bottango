// ModuleMaster.h

#ifndef _ModuleMaster_h
#define _ModuleMaster_h

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
	DataSource,
	Decoder,
	Parser,
	EffectorPool,
	StopButton,
	StatusLights,
	AudioI2S,
	Max
};

/**
 * @brief The ModuleMaster class is responsible for managing and executing the lifecycle of various modules in the system.
 * @details It handles the setup, initialization, and execution of different phases for each registered module.
 */
class ModuleMaster
{
public:

	/**
	 * @brief Default constructor for the ModuleMaster class.
	 */
	ModuleMaster() {};

	/**
	 * @brief Default destructor for the ModuleMaster class.
	 */
	~ModuleMaster() = default;

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

	template <typename T>
	T* getModule(Modules moduleType)
	{
		return static_cast<T*>(modules[(int)moduleType]);
	}

private:
	/**
	 * @brief An array of pointers to registered LoopModule instances.
	 */
	LoopModule* modules[(int)Modules::Max];
};

/**
 * @brief The InterfaceRegistry class provides a mechanism to register and retrieve interfaces for different modules.
 */
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
	 * @brief A array of void* pointers, sized to (int)Modules::Max, initialized with nullptr.
	 */
	static inline void* interfaces[(int)Modules::Max] = { nullptr };
};

#endif