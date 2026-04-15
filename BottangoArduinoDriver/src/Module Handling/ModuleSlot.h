#ifndef _ModuleSlot_h
#define _ModuleSlot_h

#include <Arduino.h>
#include <math.h>
#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"
#include "../DataSource/RS485Source.h"
#include "../DataSource/EspNowSource.h"
#include "../DataSource/WifiSource.h"
#include "../DataSource/CodeSource.h"
#include "../../BottangoArduinoModules.h"

/**
 * @brief Slot identifiers for dynamic module slots.
 * @details These are only used for compile-time size calculation via SlotSize template.
 *          NOT used for runtime service lookup (use ModuleFactory::get<T>() instead).
 */
enum class ModuleSlotType : uint8_t
{
	DataSource_Offline,
	DataSource_Secondary
};

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM) || defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
template <ModuleSlotType Slot>
struct SlotSize;
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
// Note: the sizeof() calls can be guarded with #ifdef, as the max() function takes any amount of arguments
// Any new DataSource needs to be added here, in order to make sure its size is taken into account
// This is needed, to get the correct size for the buffer in the ModuleSlot, to make sure any of the possible modules that can be placed in the slot will fit.
template <> struct SlotSize<ModuleSlotType::DataSource_Offline>
{
	static constexpr size_t value = std::max({
#ifdef USE_SD_CARD_COMMAND_STREAM
		sizeof(SdCardSource),
#endif // USE_SD_CARD_COMMAND_STREAM
#ifdef USE_CODE_COMMAND_STREAM
		sizeof(CodeSource)
#endif // USE_CODE_COMMAND_STREAM
		});
};
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
template <> struct SlotSize<ModuleSlotType::DataSource_Secondary>
{
	static constexpr size_t value = std::max({
		sizeof(RS485Source),

#ifdef RELAY_COMS_ESPNOW
		sizeof(EspNowSource),
#endif // RELAY_COMS_ESPNOW

#ifdef USE_ESP32_WIFI
		sizeof(WifiSource),
#endif // USE_ESP32_WIFI
		});
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
template <size_t MaxSize>
class ModuleSlot
{
public:
	template <typename T>
	T* place()
	{
		static_assert(sizeof(T) <= MaxSize, "Module does not fit into this slot");

		destroy();
		new (_buffer) T();
		_occupied = true;
		return reinterpret_cast<T*>(_buffer);
	}

	LoopModule* get()
	{
		return _occupied ? reinterpret_cast<LoopModule*>(_buffer) : nullptr;
	}

	void destroy()
	{
		if (_occupied)
		{
			reinterpret_cast<LoopModule*>(_buffer)->~LoopModule();
			_occupied = false;
		}
	}

private:
	// Note: The buffer needs to be aligned, otherwise it might crash
	// std::max_align_t can be exchanged for a simple "4" for the ESP32
	alignas(std::max_align_t) uint8_t _buffer[MaxSize] = {};
	bool _occupied = false;
};
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM || RELAY_SUPPORTED || USE_ESP32_WIFI
#endif // _ModuleSlot_h