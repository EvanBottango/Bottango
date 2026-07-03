#pragma once

#include <Arduino.h>
#include "ISchedulable.h"
#include "ModuleSlot.h"

// Forward declarations
// Communication chain
/*class SerialSource;
class AsciiCmdDecoder;
class Parser;
class CommandDecoder;*/

// Data sources
class DataSource;
/*class SdCardSource; */

// Offline playback
/*class AnimationPlaybackControl;*/

// Output modules
/*class StopButtonModule;
class StatusLights;
class I2SAudioModule;*/

// Relay
/*class Relay;
class EspNowSource;
class RS485Source;
class RelayESPNow;
class RelayRS485;*/

/**
 * @note Terminology:
 * - **Module**: A set of functionality enabled at the user understanding level
 * - **Service**: A singleton like functionality provider that can be swapped out at startup based on what modules the user has enabled
 * - **ISchedulable**: Anything that implements the interface, and wants to participate in the ordered init, then ordered loop phases
 */

/**
 * @brief The ServiceFactory class is responsible for creating and managing all service instances in the system.
 * @details It owns all service instances and provides service lookup functionality.
 *          The factory separates service creation/ownership from the phase-based execution loop.
 */
class ServiceFactory
{
public:
	ServiceFactory(const ServiceFactory&) = delete;
	ServiceFactory& operator=(const ServiceFactory&) = delete;

	/**
	 * @brief Creates all service instances. Is called once during system setup.
	 * @details Services are created but not yet initialized or wired together.
	 */
	static void setup();

	/**
	 * @brief Wires service dependencies together. Is called once after setup(), before init.
	 * @details Sets all necessary cross-references between services (e.g., decoder->setDataSource()).
	 */
	static void wireServices();

	/**
	 * @brief Template-based service lookup. Returns a pointer to the requested service type.
	 * @tparam T The type of the service to retrieve.
	 * @return Pointer to the requested service instance (T*), or nullptr if not found.
	 */
	template <typename T>
	static T* get()
	{
		return getInstance().get_Impl<T>();
	}

#ifdef OFFLINE_DATA_SOURCE_ENABLED
	/**
	 * @brief Places a new data source service into the offline data source slot.
	 * @details The old instance in the slot will be destroyed gracefully. The new instance is
	 *          immediately initialized.
	 * @tparam T The concrete data source service class to place in the offline slot.
	 * @return Pointer to the newly created service instance (T*).
	 */
	template <typename T>
	static T* placeInOfflineSlot()
	{
		return getInstance().placeInOfflineSlot_Impl<T>();
	}

	/**
	 * @brief Returns the currently active offline data source service.
	 * @return Pointer to the offline data source (DataSource*), or nullptr if slot is empty.
	 */
	static DataSource* getOfflineDataSource()
	{
		return getInstance().getOfflineDataSource_Impl();
	}
#endif // OFFLINE_DATA_SOURCE_ENABLED

#ifdef SECONDARY_DATA_SOURCE_ENABLED
	/**
	 * @brief Places a new data source service into the secondary data source slot.
	 * @details The old instance in the slot will be destroyed gracefully. The new instance is
	 *          immediately initialized.
	 * @tparam T The concrete data source service type to place in the secondary slot.
	 * @return Pointer to the newly created service instance (T*).
	 */
	template <typename T>
	static T* placeInSecondarySlot()
	{
		return getInstance().placeInSecondarySlot_Impl<T>();
	}

	/**
	 * @brief Returns the currently active secondary data source service.
	 * @return Pointer to the secondary data source (DataSource*), or nullptr if slot is empty.
	 */
	static DataSource* getSecondaryDataSource()
	{
		return getInstance().getSecondaryDataSource_Impl();
	}
#endif // SECONDARY_DATA_SOURCE_ENABLED

private:
	// === Core Services (always present) ===
	/*SerialSource* m_serialSource = nullptr;
	AsciiCmdDecoder* m_asciiDecoder = nullptr;
	Parser* m_parser = nullptr;*/

#ifdef OFFLINE_DATA_SOURCE_ENABLED
	// ToDo: Turned off during this step of staged refactor
	//AnimationPlaybackControl* m_animPlaybackControl = nullptr;
	ModuleSlot<SlotSize<ModuleSlotType::DataSource_Offline>::value> m_slotOfflineDataSource;
#endif

#ifdef RELAY_SUPPORTED
	// ToDo: Turned off during this step of staged refactor
	//Relay* m_relayComs = nullptr;
#endif

#ifdef RELAY_COMS_ESPNOW
	// ToDo: Turned off during this step of staged refactor
	//EspNowSource* m_espNowSource = nullptr;
#endif

#ifdef RELAY_COMS_RS485
	// ToDo: Turned off during this step of staged refactor
	//RS485Source* m_rs485Source = nullptr;
#endif

#ifdef SECONDARY_DATA_SOURCE_ENABLED
	ModuleSlot<SlotSize<ModuleSlotType::DataSource_Secondary>::value> m_slotSecondaryDataSource;
#endif

	// === Optional Modules ===
#ifdef STOP_BUTTON_SUPPORTED
	// ToDo: Turned off during this step of staged refactor
	//StopButtonModule* m_stopButton = nullptr;
#endif

#ifdef ENABLE_STATUS_LIGHTS
	// ToDo: Turned off during this step of staged refactor
	//StatusLights* m_statusLights = nullptr;
#endif

#ifdef AUDIO_SD_I2S
	// ToDo: Turned off during this step of staged refactor
	//I2SAudioModule* m_audioI2S = nullptr;
#endif

	/**
	 * @brief Default constructor for the ServiceFactory class.
	 */
	ServiceFactory() = default;

	/**
	 * @brief Default destructor for the ServiceFactory class.
	 */
	~ServiceFactory() = default;

	/**
	 * @brief Returns the singleton instance of the ServiceFactory.
	 * @return Reference to the singleton ServiceFactory instance.
	 */
	static ServiceFactory& getInstance()
	{
		static ServiceFactory instance;
		return instance;
	}

	// ==== Private Implementation Methods ====
	void setup_Impl();
	void wireModules_Impl() const;

	template <typename T>
	T* get_Impl() const;

	// ==== Offline sources ====
#ifdef OFFLINE_DATA_SOURCE_ENABLED
	template <typename T>
	T* placeInOfflineSlot_Impl()
	{
		T* moduleInstance = m_slotOfflineDataSource.place<T>();
		moduleInstance->init();

		return moduleInstance;
	}

	DataSource* getOfflineDataSource_Impl() const
	{
		// ToDo: Turned off during this step of staged refactor
		//return reinterpret_cast<DataSource*>(m_slotOfflineDataSource.get());
		return nullptr;
	}
#endif // OFFLINE_DATA_SOURCE_ENABLED

	// === Relay sources ===
#ifdef SECONDARY_DATA_SOURCE_ENABLED
	template <typename T>
	T* placeInSecondarySlot_Impl()
	{
		T* moduleInstance = m_slotSecondaryDataSource.place<T>();
		moduleInstance->init();

		return moduleInstance;
	}

	DataSource* getSecondaryDataSource_Impl() const
	{
		// ToDo: Turned off during this step of staged refactor
		//return reinterpret_cast<DataSource*>(m_slotSecondaryDataSource.get());
		return nullptr;
	}
#endif // SECONDARY_DATA_SOURCE_ENABLED

	/**
	 * @brief Creates a static instance of the specified service type.
	 * @tparam T The class of the service to create.
	 * @return Pointer to the created instance (T*).
	 */
	template <typename T>
	T* createModule();
};

// === Template Specializations for get<T>() ===
// ToDo: Turned off during this step of staged refactor
//template<> inline SerialSource* ServiceFactory::get_Impl<SerialSource>() { return m_serialSource; }
//template<> inline AsciiCmdDecoder* ServiceFactory::get_Impl<AsciiCmdDecoder>() { return m_asciiDecoder; }
//template<> inline CommandDecoder* ServiceFactory::get_Impl<CommandDecoder>() { return reinterpret_cast<CommandDecoder*>(m_asciiDecoder); }
//template<> inline Parser* ServiceFactory::get_Impl<Parser>() { return m_parser; }

#ifdef OFFLINE_DATA_SOURCE_ENABLED
// ToDo: Turned off during this step of staged refactor
//template<> inline AnimationPlaybackControl* ServiceFactory::get_Impl<AnimationPlaybackControl>() { return m_animPlaybackControl; }
#endif // OFFLINE_DATA_SOURCE_ENABLED

#ifdef RELAY_SUPPORTED
// ToDo: Turned off during this step of staged refactor
//template<> inline Relay* ServiceFactory::get_Impl<Relay>() { return m_relayComs; }
#ifdef RELAY_COMS_ESPNOW
// ToDo: Turned off during this step of staged refactor
//template<> inline EspNowSource* ServiceFactory::get_Impl<EspNowSource>() { return m_espNowSource; }
#endif
#ifdef RELAY_COMS_RS485
// ToDo: Turned off during this step of staged refactor
//template<> inline RS485Source* ServiceFactory::get_Impl<RS485Source>() { return m_rs485Source; }
#endif
#endif

#ifdef STOP_BUTTON_SUPPORTED
// ToDo: Turned off during this step of staged refactor
//template<> inline StopButtonModule* ServiceFactory::get_Impl<StopButtonModule>() { return m_stopButton; }
#endif

#ifdef ENABLE_STATUS_LIGHTS
// ToDo: Turned off during this step of staged refactor
//template<> inline StatusLights* ServiceFactory::get_Impl<StatusLights>() { return m_statusLights; }
#endif

#ifdef AUDIO_SD_I2S
// ToDo: Turned off during this step of staged refactor
//template<> inline I2SAudioModule* ServiceFactory::get_Impl<I2SAudioModule>() { return m_audioI2S; }
#endif