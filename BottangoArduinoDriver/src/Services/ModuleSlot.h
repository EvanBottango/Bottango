#pragma once

#include <Arduino.h>
#include <math.h>
// ToDo: Turned off during this step of staged refactor
/*#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"
#include "../DataSource/RS485Source.h"
#include "../DataSource/EspNowSource.h"
#include "../DataSource/WifiSource.h"
#include "../DataSource/CodeSource.h"*/
#include "../../BottangoArduinoModules.h"

/**
 * @brief Slot identifiers for dynamic module slots.
 * @details These are only used for compile-time size calculation via SlotSize template.
 *          NOT used for runtime service lookup (use ServiceFactory::get<T>() instead).
 */
enum class ModuleSlotType : uint8_t
{
	DataSource_Offline,
	DataSource_Secondary
};

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM) || defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
/**
 * @brief Template struct to calculate the size of a module slot based on its type.
 * @tparam Slot The module slot type (ModuleSlotType).
 */
template <ModuleSlotType Slot>
struct SlotSize;
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
/**
 * @brief This template struct calculates the maximum size of the module that can be placed in the DataSource_Offline slot.
 * @note The sizeof() calls can be guarded with #ifdef, as the max() function takes any amount of arguments.
 *       Any new DataSource needs to be added here, in order to make sure its size is taken into account.
 *       This is needed to get the correct size for the buffer in the ModuleSlot, to make sure any of the possible modules that can be placed in the slot will fit.
 */
template <>
struct SlotSize<ModuleSlotType::DataSource_Offline>
{
	// ToDo: Set to 2 during this step of staged refactor (0 throws compiler error)
	static constexpr size_t value = 2;
	/*static constexpr size_t value = std::max({
#ifdef USE_SD_CARD_COMMAND_STREAM
		sizeof(SdCardSource),
#endif // USE_SD_CARD_COMMAND_STREAM
#ifdef USE_CODE_COMMAND_STREAM
		sizeof(CodeSource)
#endif // USE_CODE_COMMAND_STREAM
		});*/
};
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
/**
 * @brief This template struct calculates the maximum size of the module that can be placed in the DataSource_Secondary slot.
 */
template <>
struct SlotSize<ModuleSlotType::DataSource_Secondary>
{
	// ToDo: Set to 2 during this step of staged refactor (0 throws compiler error)
	static constexpr size_t value = 2;
	/*static constexpr size_t value = std::max({
		sizeof(RS485Source),

#ifdef RELAY_COMS_ESPNOW
		sizeof(EspNowSource),
#endif // RELAY_COMS_ESPNOW

#ifdef RELAY_COMS_RS485
		sizeof(RS485Source),
#endif	// RELAY_COMS_RS485

#ifdef USE_ESP32_WIFI
		sizeof(WifiSource),
#endif // USE_ESP32_WIFI
		});*/
};
#endif // RELAY_SUPPORTED || USE_ESP32_WIFI

// Note: This also works with any other module:
/*template <> struct SlotSize<ModuleSlotType::Decoder>
{
	static constexpr size_t value = max({
		sizeof(AsciiCmdDecoder),
		sizeof(BinaryCmdDecoder)
		});
};*/

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM) || defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
/**
 * @brief This class represents a dynamic module slot that can hold a single instance of a module (e.g., a data source).
 * @tparam MaxSize The maximum size of the module that can be placed in this slot, determined by SlotSize<ModuleSlotType>::value.
 */
template <size_t MaxSize>
class ModuleSlot
{
public:
	/**
	 * @brief Places a new module of type T into the slot.
	 * @tparam T The concrete module type to place in the slot.
	 * @return A pointer to the newly created module instance (T*).
	 */
	template <typename T>
	T* place()
	{
		static_assert(sizeof(T) <= MaxSize, "Module does not fit into this slot");

		destroy();
		new (m_buffer) T();
		m_occupied = true;
		return reinterpret_cast<T*>(m_buffer);
	}

	/**
	 * @brief Returns a pointer to the currently occupied module in the slot, or nullptr if the slot is empty.
	 * @return A pointer to the module (ISchedulable*), or nullptr if the slot is empty.
	 */
	ISchedulable* get()
	{
		return m_occupied ? reinterpret_cast<ISchedulable*>(m_buffer) : nullptr;
	}

	/**
	 * @brief Calls the destructor of the currently occupied module in the slot, if any, and marks the slot as not occupied.
	 */
	void destroy()
	{
		if (m_occupied)
		{
			reinterpret_cast<ISchedulable*>(m_buffer)->~ISchedulable();
			m_occupied = false;
		}
	}

private:
	// Note: The buffer needs to be aligned, otherwise it might crash
	// std::max_align_t can be exchanged for a simple "4" for the ESP32, or "1" for AVR / 8 bit controllers, but this is more portable.
	alignas(std::max_align_t) uint8_t m_buffer[MaxSize] = {};
	bool m_occupied = false;
};
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM || RELAY_SUPPORTED || USE_ESP32_WIFI