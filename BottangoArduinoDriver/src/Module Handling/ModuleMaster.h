// ModuleMaster.h

#ifndef _ModuleMaster_h
#define _ModuleMaster_h

#include <Arduino.h>
#include "ModuleLoop.h"
#include "ModuleSlot.h"



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

	/**
	 * @brief Retrieves a module of the specified type.
	 * @tparam T The type of the module to retrieve.
	 * @param moduleType The type of module to retrieve.
	 * @return Pointer to the requested module instance.
	 */
	template <typename T>
	T* getModule(Modules moduleType)
	{
		return static_cast<T*>(modules[(int)moduleType]);
	}

	/**
	 * @brief Registers a module of the specified type.
	 * @tparam T The type of the module to register.
	 * @param moduleType The type of module to register.
	 * @return Pointer to the registered module instance.
	 */
	template <typename T>
	T* registerModule(Modules moduleType)
	{
		static T moduleInstance;
		modules[(int)moduleType] = &moduleInstance;
		return &moduleInstance;
	}

	/**
	 * @brief Register a data source into the secondary data source slot. The old instance in the slot will be destroyed gracefully using the destructor.
	 * @tparam T The concrete data source module type to place in the secondary data source slot.
	 * @return Pointer to the newly created/placed module instance (T*), which is also stored in the modules registry.
	 */
	template <typename T>
	T* registerModuleInSecondaryDataSlot()
	{
		// Note: we could add our own implementation of std::is_base_of. Or we just try to catch wrongfull use in Code Review.
		//static_assert(std::is_base_of<DataSource, T>::value, "T must be derived from DataSource");

		T* moduleInstance = slotSecondaryDataSource.place<T>();
		modules[(int)Modules::DataSource_Secondary] = moduleInstance;
		return moduleInstance;
	}

private:
	/**
	 * @brief An array of pointers to registered LoopModule instances.
	 */
	LoopModule* modules[(int)Modules::Max];

	/**
	 * @brief A module slot sized for the biggest possible secondary data source module.
	 */
	ModuleSlot<SlotSize<Modules::DataSource_Secondary>::value> slotSecondaryDataSource;
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