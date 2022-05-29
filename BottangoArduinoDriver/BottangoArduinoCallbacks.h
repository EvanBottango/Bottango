#ifndef Callbacks_h
#define Callbacks_h

class AbstractEffector;

namespace Callbacks
{
    void onEffectorRegistered(AbstractEffector *effector);
    void onEffectorDeregistered(AbstractEffector *effector);
    void effectorSignalOnLoop(AbstractEffector *effector, int signal);

    void onCurvedCustomEventMovementChanged(AbstractEffector *effector, float newMovement);
    void onOnOffCustomEventOnOffChanged(AbstractEffector *effector, bool on);
    void onTriggerCustomEventTriggered(AbstractEffector *effector);
} // namespace Callbacks

#endif