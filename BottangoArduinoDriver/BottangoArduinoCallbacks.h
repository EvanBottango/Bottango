#ifndef Callbacks_h
#define Callbacks_h

#include "Arduino.h"

class AbstractEffector;

namespace Callbacks
{
    void onThisControllerStarted();
    void onThisControllerStopped();
    void onLateLoop();
    void onEarlyLoop();

    void onEffectorRegistered(AbstractEffector *effector);
    void onEffectorDeregistered(AbstractEffector *effector);
    void effectorSignalOnLoop(AbstractEffector *effector, int signal);

    void onCurvedCustomEventMovementChanged(AbstractEffector *effector, float newMovement);
    void onOnOffCustomEventOnOffChanged(AbstractEffector *effector, bool on);
    void onTriggerCustomEventTriggered(AbstractEffector *effector);
    void onColorCustomEventColorChanged(AbstractEffector *effector, byte newRed, byte newGreen, byte newBlue);
    bool isStepperAutoHomeComplete(AbstractEffector *effector);
} // namespace Callbacks

#endif