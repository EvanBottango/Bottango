#include "BottangoArduinoCallbacks.h"
#include "src/AbstractEffector.h"
#include "src/Log.h"

namespace Callbacks
{
    // All effectors have an identifier. It is an 8 char or less string. Check Bottango to see the identifier for a given effector in app.
    // for most effectors, it is the first pin in their set of pins
    // i2c effectors have the i2c address before the first pin
    // you query for an effector with a c string char array, instanitated at 9 characters (8 for the identifier, and a null terminating char)

    // !!!!!!!!!!!!!!! //
    // !! CALLBACKS !! //
    // !!!!!!!!!!!!!!! //
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
    // !!NOTE!! In order to keep interrupt driven effectors (Pin Stepper and Step/Dir Stepper) doing as little as possible in the interrupt, this is called on loop. That means you're not going to get a call every step, just a check in each loop frame of the current signal, on interrupt effectors.
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
} // namespace Callbacks