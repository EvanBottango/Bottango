#pragma once

#include <Arduino.h>
#include "../../BottangoArduinoModules.h"


class OutgoingBase
{
public:
	// ToDo: These helper functions should be moved to somewhere else, this feels more of a protocol type of thing, not a pure "outgoing" thing.
	/** User request to start playing animation with index and time in MS*/
	void outgoing_requestStartPlay(int animationIndex, unsigned long startTime);

	/** User request to start playing animation in current state*/
	void outgoing_requestStartPlay();

#ifdef ONLINE_BUTTON_ACTIONS
	void outgoing_requestStartPlayViaButton(int btnIdex);
#endif

	/**
	 * @brief Prints a string from memory to the outgoing channel. The string must be null-terminated.
	 * @param targetOutput The string to print, stored in memory (RAM). It must be null-terminated.
	 */
	void printOutputStringMem(const char* targetOutput);

	/**
	 * @brief Various overloads to print different types of values from memory to the outgoing channel. Each overload converts the value to a string and prints it.
	 * @param value The value to print, which can be of type int, long, bool, char, uint16_t, or uint32_t.
	 */
	void printOutputStringMem(int value);
	void printOutputStringMem(long value);
	void printOutputStringMem(bool value);
	void printOutputStringMem(char value);
	void printOutputStringMem(uint16_t value);
	void printOutputStringMem(uint32_t value);

	/**
	 * @brief Prints a string from program memory (PROGMEM) to the outgoing channel. The string must be null-terminated.
	 * @param targetOutput The string to print, stored in program memory (PROGMEM). It must be null-terminated.
	 */
	void printOutputStringPROGMEM(const char* targetOutput);

	/**
	 * @brief Prints a string from flash memory to the outgoing channel. The string must be null-terminated.
	 * @param str The string to print, stored in flash memory. It must be null-terminated.
	 */
	void printOutputStringFlash(const __FlashStringHelper* str);

	/**
	 * @brief Prints a newline character to the outgoing channel
	 */
	void printLine();

	/**
	 * @brief Flushes the outgoing channel, ensuring that all buffered data is sent.
	 */
	void flush();

protected:
	virtual void printStringFlash_Implementation(const __FlashStringHelper* str) {};
	virtual void printStringMem_Implementation(const char* str) {};
	virtual void printLine_Implementation() {};
	virtual void flush_Implementation() {};
};

/**
 * @brief Generic static accessor for any outgoing channel.
 * Static forwarding methods are defined ONCE here.
 * How to add a new functionality:
 * New channel -> Add new OutgoingTag struct + using alias.
 * New method  -> Add in OutgoingChannelAccessor and in OutgoingBase.
 * @tparam Tag Empty marker struct that makes each instantiation unique.
 */
template<typename Tag>
class OutgoingChannelAccessor
{
public:
	static void bind(OutgoingBase* p) { m_s_ptr = p; }
	static OutgoingBase* get() { return m_s_ptr; }

	static void printOutputStringMem(const char* str) { if (m_s_ptr) m_s_ptr->printOutputStringMem(str); }
	static void printOutputStringMem(int v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(long v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(bool v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(char v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(uint16_t v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(uint32_t v) { if (m_s_ptr) m_s_ptr->printOutputStringMem(v); }
	static void printOutputStringPROGMEM(const char* str) { if (m_s_ptr) m_s_ptr->printOutputStringPROGMEM(str); }
	static void printOutputStringFlash(const __FlashStringHelper* str) { if (m_s_ptr) m_s_ptr->printOutputStringFlash(str); }
	static void printLine() { if (m_s_ptr) m_s_ptr->printLine(); }
	static void flush() { if (m_s_ptr) m_s_ptr->flush(); }

private:
	static inline OutgoingBase* m_s_ptr = nullptr;
};

// Channel tags (empty marker structs)
namespace OutgoingTag
{
	struct Host {};
	struct SerialPort {};
	struct Relay {};
}

// Channel aliases – add new channels here as needed
// Usage: Use Outgoing::printLine() for the currently active 'host' channel, or OutgoingSerial::printLine() to specifically target the serial channel, etc.
// Use Outgoing::bind(OutgoingRelay::get()) to set the relay as 'host' channel
// 'Host' was choosen over 'primary' for naming, because the modules use 'Primary' to refer to the serial data source.
// 'Host' referes to the main output channel back to the host, which can be switched between Serial, Relay, or others in the future.
using OutgoingR = OutgoingChannelAccessor<OutgoingTag::Host>;
using OutgoingSerial = OutgoingChannelAccessor<OutgoingTag::SerialPort>;

#if defined(RELAY_SUPPORTED)
using OutgoingRelay = OutgoingChannelAccessor<OutgoingTag::Relay>;
#endif // RELAY_SUPPORTED