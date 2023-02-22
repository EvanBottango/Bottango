
#include <Arduino.h>

namespace Error
{
    void reportError_NoServoOnPin()
    {
        Serial.println();
        Serial.println(F("errNoServoOnPin"));
    }

    void reportError_ServoCollision()
    {
        Serial.println();
        Serial.println(F("errServoCollision"));
    }

    void reportError_NoSpaceAvailable()
    {
        Serial.println();
        Serial.println(F("errNoSpaceAvailable"));
    }

    void reportError_CmdTooLong()
    {
        Serial.println();
        Serial.println(F("errCmdTooLong"));
    }

    void reportError_TooManyCommands()
    {
        Serial.println();
        Serial.println(F("errTooManyCommands"));
    }

    void reportError_TooManyParams()
    {
        Serial.println();
        Serial.println(F("errTooManyParams"));
    }

    void reportError_MissingLibrary()
    {
        Serial.println();
        Serial.println(F("errMissingLibrary"));
    }

    void reportError_TooManyI2c()
    {
        Serial.println();
        Serial.println(F("errI2COverrun"));
    }

    void reportError_InvalidPin()
    {
        Serial.println();
        Serial.println(F("errInvalidPin"));
    }

} // namespace Error
