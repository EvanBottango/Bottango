#ifndef _ModuleMaster_h
#define _ModuleMaster_h

#include <Arduino.h>
#include "LoopModule.h"
#include "ModuleSlot.h"

/**
 * @brief The ModuleMaster class is responsible for managing and executing the lifecycle of various modules/functions in the system.
 * @details It handles the setup, initialization, and execution of different phases for each registered module.
 */
class ModuleMaster
{
public:

	/**
	 * @brief Default constructor for the ModuleMaster class.
	 */
	ModuleMaster() = default;

	/**
	 * @brief Default destructor for the ModuleMaster class.
	 */
	~ModuleMaster() = default;

	/**
	 * @brief Initial setup of modules. Modules are created and registered in this method. Is called once during system setup.
	 */
	void setupModules();

	/**
	 * @brief Initializes every module, which were set up earlier. Is called once during system setup.
	 */
	void initModules() const;

	/**
	 * @brief Executes the specified phase. Loops through all registered modules and calls their onPhase method.
	 * @param p The phase to execute.
	 */
	void executePhase(Phase const p) const;

	/**
	 * @brief Retrieves a specific module instance by its enum representation. The returned pointer is of the type specified by the template parameter T.
	 * @tparam T The class of the module to retrieve. Can be a base class, that the module inherits from.
	 * @param moduleType The enum entry that corresponds to the module being requested.
	 * @return Pointer to the requested module instance (T*).
	 */
	template <typename T>
	T* getModule(Modules moduleType)
	{
		return static_cast<T*>(_modules[static_cast<int>(moduleType)]);
	}

	/**
	 * @brief Creates a static instance of the specified module type and registers it in the modules registry.
	 * Static storage duration is used for the module instance, so it will be automatically destroyed when the program ends.
	 * @tparam T The class of the module to register.
	 * @param moduleType The enum entry that corresponds to the module being registered.
	 * @return Pointer to the registered instance (T*).
	 */
	template <typename T>
	T* registerModule(Modules moduleType)
	{
		static T moduleInstance;
		_modules[static_cast<int>(moduleType)] = &moduleInstance;
		return &moduleInstance;
	}

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	/**
	 * @brief Register a data source into the offline data source slot and initialize it by calling init(). The old instance in the slot will be destroyed gracefully using the destructor.
	 * Static storage duration is used for the module instance, so it will be automatically destroyed when the program ends.
	 * @tparam T The concrete data source module class to place in the offline data source slot.
	 * @return Pointer to the newly created/placed module instance (T*).
	 */
	template <typename T>
	T* registerModuleInOfflineDataSlot()
	{
		// Note: we could add our own implementation of std::is_base_of. Or we just try to catch wrongfull use in Code Review.
		//static_assert(std::is_base_of<DataSource, T>::value, "T must be derived from DataSource");

		T* moduleInstance = _slotOfflineDataSource.place<T>();

		// Call init immediately to ensure the module is ready for use right after registration
		moduleInstance->init(); 
		_modules[static_cast<int>(Modules::DataSource_Offline)] = moduleInstance;
		return moduleInstance;
	}
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	/**
	 * @brief Register a data source into the secondary data source slot and initialize it by calling init(). The old instance in the slot will be destroyed gracefully using the destructor.
	 * Static storage duration is used for the module instance, so it will be automatically destroyed when the program ends.
	 * @tparam T The concrete data source module type to place in the secondary data source slot.
	 * @return Pointer to the newly created/placed module instance (T*).
	 */
	template <typename T>
	T* registerModuleInSecondaryDataSlot()
	{
		// Note: we could add our own implementation of std::is_base_of. Or we just try to catch wrongfull use in Code Review.
		//static_assert(std::is_base_of<DataSource, T>::value, "T must be derived from DataSource");

		T* moduleInstance = _slotSecondaryDataSource.place<T>();

		// Call init immediately to ensure the module is ready for use right after registration
		moduleInstance->init();
		_modules[static_cast<int>(Modules::DataSource_Secondary)] = moduleInstance;
		return moduleInstance;
	}
#endif // RELAY_SUPPORTED || USE_ESP32_WIFI

private:
	/**
	 * @brief An array of pointers to registered LoopModule instances.
	 */
	LoopModule* _modules[static_cast<int>(Modules::Max)] = {};

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	/**
	 * @brief A module slot sized for the biggest possible offline data source module.
	 */
	ModuleSlot<SlotSize<Modules::DataSource_Offline>::value> _slotOfflineDataSource;
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	/**
	 * @brief A module slot sized for the biggest possible secondary data source module.
	 */
	ModuleSlot<SlotSize<Modules::DataSource_Secondary>::value> _slotSecondaryDataSource;
#endif // RELAY_SUPPORTED || USE_ESP32_WIFI
};

#endif // _ModuleMaster_h