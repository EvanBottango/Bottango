#pragma once

#include <Arduino.h>
#include "../../BottangoArduinoModules.h"

namespace Outgoing
{
	/** Request Shutdown */
	const char REQ_SHUTDOWN[] PROGMEM = "reqStop\n";

	/** Request pause anim */
	const char STOP_PLAY[] PROGMEM = "reqPause\n";

	/** Request start anim */
	const char START_PLAY[] PROGMEM = "reqPlay,";

#ifdef ONLINE_BUTTON_ACTIONS
	const char START_PLAY_BUTTON[] PROGMEM = "reqPlayBtn,";
#endif

	/** Stepper/Custom Motor Auto Sync is Complete */
	const char SYNC_COMPLETE[] PROGMEM = "sycMDone,";

	/** User request to shutdown */
	void outgoing_requestShutdown();

	/** User request to stop playing animation */
	void outgoing_requestStopPlay();

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

#ifdef RELAY_SUPPORTED
	void setSecondaryPeerOutgoing(bool enabled);
	void toggleOnSecondaryOutgoing();
	void endToggleOnSecondaryOutgoing();
	extern bool ignoreToggleOff;
	extern bool secondaryPeerOutgoing;
#endif

} // namespace Outgoing