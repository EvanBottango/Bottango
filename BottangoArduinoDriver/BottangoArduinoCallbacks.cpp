#include "BottangoArduinoCallbacks.h"
#include "src/AbstractEffector.h"
#include "src/Log.h"
#include "src/Outgoing.h"

namespace Callbacks
{
    // !!!!!!!!!!!!!!! //
    // !! CONTROLLER LIFECYCLE CALLBACKS !! //
    // !!!!!!!!!!!!!!! //

    // called AFTER a successful handshake with the Bottango application, signifying that this controller has started.
    // use for general case startup process
    // Effector registration will happen after this callback, in their own callback.
    // If you have effector registration specific needs, you should use onEffectorRegistered
    void onThisControllerStarted()
    {
    }

    // called after the controller recieves a stop command. The controller will stop all movement, deregister all effectors
    // After which this call back is triggered.
    void onThisControllerStopped()
    {
    }

    // called each loop cycle. If you have timing based code you'd like to utilize outside of the Bottango animation
    // This callback occurs BEFORE all effectors process their movement, at the end of the loop.
    void onEarlyLoop()
    {
    }

    // called each loop cycle.
    // This callback occurs AFTER all effectors process their movement, at the end of the loop.
    void onLateLoop()
    {
        
        // EX: Request stop on driver, and disconnect all active connections
        // Outgoing::outgoing_requestEStop();
        
        // EX: Pause Playing in App
        // Outgoing::outgoing_requestStopPlay();
        
        // EX: Start Playing in App (in current animation and time)
        // Outgoing::outgoing_requestStartPlay();
        
        // EX: Start Playing in App (with animation index, and start time in milliseconds)
        // Outgoing::outgoing_requestStartPlay(1,1000);
    }

    // !!!!!!!!!!!!!!! //
    // !! EFFECTOR CALLBACKS !! //
    // !!!!!!!!!!!!!!! //

    // All effectors have an identifier. It is an 8 char or less string. Check Bottango to see the identifier for a given effector in app.
    // for most effectors, it is the first pin in their set of pins
    // i2c effectors have the i2c address before the first pin
    // you query for an effector with a c string char array, instanitated at 9 characters (8 for the identifier, and a null terminating char)

    // The below are called by built in effectors at various stages in their lifecycle

    // called by an effector when registered, after registration is complete
    void onEffectorRegistered(AbstractEffector *effector)
    {
        // example, turn on built in LED if effector registered with identifier "1";

        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "1") == 0)
        // {
        //     pinMode(LED_BUILTIN, OUTPUT);
        //     digitalWrite(LED_BUILTIN, HIGH);
        // }
    }

    // called by an effector when deregistered, before deregistration is complete
    void onEffectorDeregistered(AbstractEffector *effector)
    {
        // example, turn off built in LED if effector registered with identifier "1";

        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "1") == 0)
        // {
        //     pinMode(LED_BUILTIN, OUTPUT);
        //     digitalWrite(LED_BUILTIN, LOW);
        // }
    }

    // called by effectors each loop with its current signal (example: servo PWM or stepper steps from home )
    void effectorSignalOnLoop(AbstractEffector *effector, int signal)
    {
        // example, set built in led for effector with identifier "1" based on if signal is greater than 1500

        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "1") == 0)
        // {
        //     pinMode(LED_BUILTIN, OUTPUT);
        //     if (signal > 1500)
        //     {
        //         digitalWrite(LED_BUILTIN, HIGH);
        //     }
        //     else
        //     {
        //         digitalWrite(LED_BUILTIN, LOW);
        //     }
        // }

        // another example, drive a custom motor, which you have coded to have a setSignal function
        // if (strcmp(effectorIdentifier, "myMotor") == 0)
        // {
        //      myMotor->setSignal(signal);
        // }
    }

    // !!!!!!!!!!!!!!!!!!! //
    // !! CUSTOM EVENTS !! //
    // !!!!!!!!!!!!!!!!!!! //
    // The below are called by custom events so you can provide your own behaviours

    // called by a curved custom event any time the movement value is changed during a curved movement. (Movement is a normalized float between 0.0 - 1.0)
    void onCurvedCustomEventMovementChanged(AbstractEffector *effector, float newMovement)
    {
        // example, fade an led based on the new movement value
        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "myLight") == 0)
        // {
        //     pinMode(5, OUTPUT);
        //     int brightness = 255 * newMovement;
        //     analogWrite(5, brightness);
        // }
    }

    // called by a on off custom event any time the on off value is changed.
    void onOnOffCustomEventOnOffChanged(AbstractEffector *effector, bool on)
    {
        // example, turn on built in led based on the on off value
        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "myLight") == 0)
        // {
        //     pinMode(LED_BUILTIN, OUTPUT);
        //     digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
        // }
    }

    // called by a trigger custom event any time the on event is triggered.
    void onTriggerCustomEventTriggered(AbstractEffector *effector)
    {
        // example, set led to a random brightness each trigger
        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "myLight") == 0)
        // {
        //     pinMode(5, OUTPUT);
        //     int brightness = random(0, 256);
        //     analogWrite(5, brightness);
        // }
    }

    void onColorCustomEventColorChanged(AbstractEffector *effector, byte newRed, byte newGreen, byte newBlue)
    {
        // example, set rgb LED on pins 3, 5, and 6 to given red, green, and blue colors (represented as a byte between 0 and 255)
        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "myRGB") == 0)
        // {
        //     pinMode(3, OUTPUT);
        //     pinMode(5, OUTPUT);
        //     pinMode(6, OUTPUT);

        //     analogWrite(3, newRed);
        //     analogWrite(5, newGreen);
        //     analogWrite(6, newBlue);
        // }

        // code free support for addressable LED's (neopixel, etc. coming soon)
        // in the meanwhile, get support in the Bottango discord channel for "how to" info
    }

    bool isStepperAutoHomeComplete(AbstractEffector *effector)
    {
        // return true if the given stepper is at home position
        // else return false

        // example, end homing on stepper with step on pin 6, when pin 10 is read high
        // char effectorIdentifier[9];
        // effector->getIdentifier(effectorIdentifier, 9);

        // if (strcmp(effectorIdentifier, "6") == 0)
        // {
        //     pinMode(10, INPUT);
        //     if (digitalRead(10) == HIGH)
        //     {
        //         return true;
        //     }
        // }

        return false;
    }
} // namespace Callbacks