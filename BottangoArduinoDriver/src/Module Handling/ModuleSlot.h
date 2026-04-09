// ModuleSlot.h

#ifndef _ModuleSlot_h
#define _ModuleSlot_h

#include <Arduino.h>
#include <math.h>
#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"
#include "../DataSource/RS485Source.h"
#include "../DataSource/EspNowSource.h"
#include "../DataSource/WifiSource.h"

/**
 * @brief Enumeration of available modules.
 */
 // Note: There are two ways of doing this. We can use this enum class, or we can use a registration system where modules register themselves.
 // The second approach would need a Map<> to have a way to look up modules by name or type.
 // First approach is simple and straight forward, but requires updating this enum class whenever a new module is added.
 // Second approach is more flexible and extensible, but adds complexity.
enum class Modules : uint8_t
{
	DataSource_Serial,			// [Mandatory] Primary data source, always present and active
	DataSource_Offline,			// [Optinal] Offline data source, can be switched between different modules (e.g. SD card, Export to code)
	DataSource_Secondary,		// [Optinal] Secondary data source, can be switched between different modules (e.g. ESP-Now, RS485, etc.)
	RelayComs,					// [Optinal] Relay communication module, if supported
	Decoder,					// [Mandatory] Command decoder, parses raw data into commands. Can be switched between different decoders (e.g. ASCII, binary, etc.)
	Parser,						// [Mandatory] Command parser, takes parsed commands and executes them
	EffectorPool,				// [Mandatory] Pool of effectors that can be used by commands	
	StopButton,					// [Optinal] Stop button module, if supported
	StatusLights,				// [Optinal] Status lights module, if supported
	AudioI2S,					// [Optinal] I2S audio module, if supported	
	AnimPlaybackCntrl,			// [Mandatory] [Has to be second to last] Animation playback control module, is used automatically, depending on the active BottangoArduinoModules
	Max							// [Mandatory] [Has to be the last] Sentinel value to indicate the number of modules, must always be last
};

template <Modules slot>
struct SlotSize;

// Note: the sizeof() calls can be guarded with #ifdef, as the max() function takes any amount of arguments
// Any new DataSource needs to be added here, in order to make sure its size is taken into account
// This is needed, to get the correct size for the buffer in the ModuleSlot, to make sure any of the possible modules that can be placed in the slot will fit.
template <> struct SlotSize<Modules::DataSource_Offline>
{
	static constexpr size_t value = std::max({
		sizeof(SdCardSource),
		//sizeof(ExportedCodeSource) // ToDo for USE_CODE_COMMAND_STREAM
		});
};

template <> struct SlotSize<Modules::DataSource_Secondary>
{
	static constexpr size_t value = std::max({
		sizeof(RS485Source),

#if defined(RELAY_COMS_ESPNOW)
		sizeof(EspNowSource),
#endif // RELAY_COMS_ESPNOW

#if defined(USE_ESP32_WIFI)
		sizeof(WifiSource),
#endif // USE_ESP32_WIFI
		});
};

// Note: This also works with any other module:
/*template <> struct SlotSize<Modules::Decoder>
{
	static constexpr size_t value = max({
		sizeof(AsciiCmdDecoder),
		sizeof(BinaryCmdDecoder)
		});
};*/

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

	LoopModule* get() const
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
	alignas(std::max_align_t) uint8_t _buffer[MaxSize];
	bool _occupied = false;
};

#endif // _ModuleSlot_h