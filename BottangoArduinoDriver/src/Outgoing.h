
#ifndef BOTTANGO_OUTGOING_H
#define BOTTANGO_OUTGOING_H

#include <Arduino.h>

namespace Outgoing
{
    /** User request to e stop */
    void outgoing_requestEStop();

    /** User request to stop playing animation */
    void outgoing_requestStopPlay();

    /** User request to start playing animation with index and time in MS*/
    void outgoing_requestStartPlay(int animationIndex, unsigned long startTime);

    /** User request to start playing animation in current state*/
    void outgoing_requestStartPlay();

    /** User request to notify that a stepper is now syncronized*/
    void outgoing_notifySyncComplete();

    /** direct output char array string*/
    void printOutputStringMem(const char *targetOutput);

    /** direct output char array string with helper casting*/
    void printOutputStringMem(int value);
    void printOutputStringMem(long value);
    void printOutputStringMem(bool value);
    void printOutputStringMem(uint16_t value);
    void printOutputStringMem(uint32_t value);

    /** direct output progmem char array string*/
    void printOutputStringPROGMEM(const char *targetOutput);

    /** direct output flash string*/
    void printOutputStringFlash(const __FlashStringHelper *str);

    void printLine();

} // namespace Outgoing

#endif // BOTTANGO_OUTGOING_H
