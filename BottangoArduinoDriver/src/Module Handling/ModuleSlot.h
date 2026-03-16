// ModuleSlot.h

#ifndef _ModuleSlot_h
#define _ModuleSlot_h

#include <Arduino.h>
#include <math.h>
#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"
#include "../DataSource/RS485Source.h"

/**
 * @brief Enumeration of available modules.
 */
 // Note: There are two ways of doing this. We can use this enum class, or we can use a registration system where modules register themselves.
 // The second approach would need a Map<> to have a way to look up modules by name or type.
 // First approach is simple and straight forward, but requires updating this enum class whenever a new module is added.
 // Second approach is more flexible and extensible, but adds complexity.
enum class Modules : uint8_t
{
	DataSource_Serial,
	DataSource_Secondary,
	Decoder,
	Parser,
	EffectorPool,
	StopButton,
	StatusLights,
	AudioI2S,
	Max
};

template <Modules slot>
struct SlotSize;

// Note: the sizeof() calls can be guarded with #ifdef, as the max() function takes any amount of arguments
// Any new DataSource needs to be added here, in order to make sure its size is taken into account
// This is needed, to get the correct size for the buffer in the ModuleSlot, to make sure any of the possible modules that can be placed in the slot will fit.
template <> struct SlotSize<Modules::DataSource_Secondary>
{
	static constexpr size_t value = std::max({
		sizeof(SdCardSource),
		sizeof(RS485Source)
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
		static_assert(sizeof(T) <= MaxSize,"Module does not fit into this slot");

		destroy();
		new (buffer) T();
		occupied = true;
		return reinterpret_cast<T*>(buffer);
	}

	LoopModule* get() const
	{
		return occupied ? reinterpret_cast<LoopModule*>(buffer) : nullptr;
	}

	void destroy()
	{
		if (occupied)
		{
			reinterpret_cast<LoopModule*>(buffer)->~LoopModule();
			occupied = false;
		}
	}

private:
	// Note: The buffer needs to be aligned, otherwise it might crash
	// std::max_align_t can be exchanged for a simple "4" for the ESP32
	alignas(std::max_align_t) uint8_t buffer[MaxSize];
	bool occupied = false;
};

#endif // _ModuleSlot_h