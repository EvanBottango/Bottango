
#include <Arduino.h>
#include "Outgoing.h"
#include "../BottangoArduinoModules.h"

namespace Error
{
    void reportError_NoServoOnPin()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errNoServoOnPin"));
        Outgoing::printLine();
    }

    void reportError_ServoCollision()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errServoCollision"));
        Outgoing::printLine();
    }

    void reportError_NoSpaceAvailable()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errNoSpaceAvailable"));
        Outgoing::printLine();
    }

    void reportError_CmdTooLong()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errCmdTooLong"));
        Outgoing::printLine();
    }

    void reportError_TooManyCommands()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errTooManyCommands"));
        Outgoing::printLine();
    }

    void reportError_TooManyParams()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errTooManyParams"));
        Outgoing::printLine();
    }

    void reportError_MissingLibrary()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errMissingLibrary"));
        Outgoing::printLine();
    }

    void reportError_TooManyI2c()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errI2COverrun"));
        Outgoing::printLine();
    }

    void reportError_InvalidPin()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errInvalidPin"));
        Outgoing::printLine();
    }

#ifdef RELAY_PARENT
    void reportWarning_RelayTimeout(const char *ident)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("warnRelayTO,"));
        Outgoing::printOutputStringMem(ident);
        Outgoing::printLine();
    }
#endif

} // namespace Error
