#pragma once

#include <Arduino.h>
#include "OutgoingSerial.h"

#include "../BottangoArduinoModules.h"

#if defined(RELAY_SUPPORTED)
#include "OutgoingRelay.h"
#endif // RELAY_SUPPORTED

class OutgoingBase
{
public:
	// ToDo: Das sind helfer, die gehören hier nicht hin
	/** User request to start playing animation with index and time in MS*/
	void outgoing_requestStartPlay(int animationIndex, unsigned long startTime);

	/** User request to start playing animation in current state*/
	void outgoing_requestStartPlay();

#ifdef ONLINE_BUTTON_ACTIONS
	void outgoing_requestStartPlayViaButton(int btnIdex);
#endif

	/** User request to notify that a stepper is now syncronized*/
	void outgoing_notifySyncComplete();

	/** direct output char array string*/
	void printOutputStringMem(const char* targetOutput);

	/** direct output char array string with helper casting*/
	void printOutputStringMem(int value);
	void printOutputStringMem(long value);
	void printOutputStringMem(bool value);
	void printOutputStringMem(char value);
	void printOutputStringMem(uint16_t value);
	void printOutputStringMem(uint32_t value);

	/** direct output progmem char array string*/
	void printOutputStringPROGMEM(const char* targetOutput);

	/** direct output flash string*/
	void printOutputStringFlash(const __FlashStringHelper* str);

	void printLine();

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
 * New channel  → new Tag struct + using alias (2 lines).
 * New method   → add here + in Outgoing (2 places).
 * @tparam Tag  Empty marker struct that makes each instantiation unique.
 */
template<typename Tag>
class OutgoingChannelAccessor
{
public:
	static void bind(OutgoingBase* p) { _s_ptr = p; }
	static OutgoingBase* get() { return _s_ptr; }

	static void printOutputStringMem(const char* str) { if (_s_ptr) _s_ptr->printOutputStringMem(str); }
	static void printOutputStringMem(int v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(long v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(bool v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(char v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(uint16_t v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringMem(uint32_t v) { if (_s_ptr) _s_ptr->printOutputStringMem(v); }
	static void printOutputStringPROGMEM(const char* str) { if (_s_ptr) _s_ptr->printOutputStringPROGMEM(str); }
	static void printOutputStringFlash(const __FlashStringHelper* str) { if (_s_ptr) _s_ptr->printOutputStringFlash(str); }
	static void printLine() { if (_s_ptr) _s_ptr->printLine(); }
	static void flush() { if (_s_ptr) _s_ptr->flush(); }

private:
	static inline OutgoingBase* _s_ptr = nullptr;
};

// Channel tags (empty marker structs)
namespace OutgoingTag { struct Host {}; struct Serial {}; struct Relay {}; }

// Channel aliases – add new channels here as needed
// Usage: Use Outgoing::printLine() for the currently active 'host' channel, or OutgoingSerial::printLine() to specifically target the serial channel, etc.
// Use Outgoing::bind(OutgoingRelay::get()) to set the relay as 'host' channel
// Host was choosen over primary for naming, because the Modules use "Primary" to refer to the serial data source.
// 'Host' referes to the main output channel back to the host, which can be switched between Serial, Relay, or others in the future.
using Outgoing = OutgoingChannelAccessor<OutgoingTag::Host>;
using OutgoingSerial = OutgoingChannelAccessor<OutgoingTag::Serial>;
using OutgoingRelay = OutgoingChannelAccessor<OutgoingTag::Relay>;