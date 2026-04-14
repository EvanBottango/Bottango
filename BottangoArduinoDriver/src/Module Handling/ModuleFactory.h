#ifndef _ModuleFactory_h
#define _ModuleFactory_h

#include <Arduino.h>
#include "LoopModule.h"
#include "ModuleSlot.h"

// Forward declarations
class SerialSource;
class AsciiCmdDecoder;
class Parser;
class AnimationPlaybackControl;
class RelayESPNow;
class StopButtonModule;
class StatusLightsModule;
class I2SAudioModule;

/**
 * @brief The ModuleFactory class is responsible for creating and managing all module instances in the system.
 * @details It owns all module instances and provides service lookup functionality.
 *          The factory separates module creation/ownership from the phase-based execution loop.
 */
class ModuleFactory
{
public:
	/**
	 * @brief Default constructor for the ModuleFactory class.
	 */
	ModuleFactory() = default;

	/**
	 * @brief Default destructor for the ModuleFactory class.
	 */
	~ModuleFactory() = default;

	/**
	 * @brief Creates all module instances. Is called once during system setup.
	 * @details Modules are created but not yet initialized or wired together.
	 */
	void setup();

	/**
	 * @brief Wires module dependencies together. Is called once after setup(), before init.
	 * @details Sets all necessary cross-references between modules (e.g., decoder->setDataSource()).
	 */
	void wireModules();

	/**
	 * @brief Template-based service lookup. Returns a pointer to the requested module type.
	 * @tparam T The type of the module to retrieve.
	 * @return Pointer to the requested module instance (T*), or nullptr if not found.
	 */
	template <typename T>
	T* get();

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	/**
	 * @brief Places a new data source module into the offline data source slot.
	 * @details The old instance in the slot will be destroyed gracefully. The new instance is
	 *          immediately initialized and wired.
	 * @tparam T The concrete data source module class to place in the offline slot.
	 * @return Pointer to the newly created module instance (T*).
	 */
	template <typename T>
	T* placeInOfflineSlot();

	/**
	 * @brief Returns the currently active offline data source module.
	 * @return Pointer to the offline data source (LoopModule*), or nullptr if slot is empty.
	 */
	LoopModule* getOfflineDataSource();
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	/**
	 * @brief Places a new data source module into the secondary data source slot.
	 * @details The old instance in the slot will be destroyed gracefully. The new instance is
	 *          immediately initialized and wired.
	 * @tparam T The concrete data source module type to place in the secondary slot.
	 * @return Pointer to the newly created module instance (T*).
	 */
	template <typename T>
	T* placeInSecondarySlot();

	/**
	 * @brief Returns the currently active secondary data source module.
	 * @return Pointer to the secondary data source (LoopModule*), or nullptr if slot is empty.
	 */
	LoopModule* getSecondaryDataSource();
#endif // RELAY_SUPPORTED || USE_ESP32_WIFI

private:
	/**
	 * @brief Creates a static instance of the specified module type.
	 * @tparam T The class of the module to create.
	 * @return Pointer to the created instance (T*).
	 */
	template <typename T>
	T* createModule();

	// === Core Modules (always present) ===
	SerialSource* _serialSource = nullptr;
	AsciiCmdDecoder* _asciiDecoder = nullptr;
	Parser* _parser = nullptr;

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	AnimationPlaybackControl* _animPlaybackControl = nullptr;
	ModuleSlot<SlotSize<Modules::DataSource_Offline>::value> _slotOfflineDataSource;
#endif

#ifdef RELAY_SUPPORTED
	RelayESPNow* _relayComs = nullptr;
#endif

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	ModuleSlot<SlotSize<Modules::DataSource_Secondary>::value> _slotSecondaryDataSource;
#endif

	// === Optional Modules ===
#ifdef STOP_BUTTON_SUPPORTED
	StopButtonModule* _stopButton = nullptr;
#endif

#ifdef ENABLE_STATUS_LIGHTS
	StatusLightsModule* _statusLights = nullptr;
#endif

#ifdef AUDIO_SD_I2S
	I2SAudioModule* _audioI2S = nullptr;
#endif
};

// === Template Specializations for get<T>() ===

template<> inline SerialSource* ModuleFactory::get<SerialSource>() { return _serialSource; }
template<> inline AsciiCmdDecoder* ModuleFactory::get<AsciiCmdDecoder>() { return _asciiDecoder; }
template<> inline Parser* ModuleFactory::get<Parser>() { return _parser; }

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
template<> inline AnimationPlaybackControl* ModuleFactory::get<AnimationPlaybackControl>() { return _animPlaybackControl; }
#endif

#ifdef RELAY_SUPPORTED
template<> inline RelayESPNow* ModuleFactory::get<RelayESPNow>() { return _relayComs; }
#endif

#ifdef STOP_BUTTON_SUPPORTED
template<> inline StopButtonModule* ModuleFactory::get<StopButtonModule>() { return _stopButton; }
#endif

#ifdef ENABLE_STATUS_LIGHTS
template<> inline StatusLightsModule* ModuleFactory::get<StatusLightsModule>() { return _statusLights; }
#endif

#ifdef AUDIO_SD_I2S
template<> inline I2SAudioModule* ModuleFactory::get<I2SAudioModule>() { return _audioI2S; }
#endif

#endif // _ModuleFactory_h