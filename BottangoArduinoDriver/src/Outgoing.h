
#ifndef BOTTANGO_OUTGOING_H
#define BOTTANGO_OUTGOING_H

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

} // namespace Outgoing

#endif // BOTTANGO_OUTGOING_H
