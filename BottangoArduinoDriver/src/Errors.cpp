
#include <Arduino.h>
#include "Outgoing.h"
#include "../BottangoArduinoModules.h"

#ifdef RELAY_SUPPORTED
#include "UDIDHelper.h"
#endif
namespace Error
{
    void reportError_NoEffectorOnPin(const char *identifer)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errNoEffectorOnPin: "));
        Outgoing::printOutputStringMem(identifer);
        Outgoing::printLine();
    }

    void reportError_EffectorCollision(const char *identifer)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errServoCollision: "));
        Outgoing::printOutputStringMem(identifer);
        Outgoing::printLine();
    }

    void reportError_NoSpaceAvailable()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errNoSpaceAvailable"));
        Outgoing::printLine();
    }

    void reportError_InvalidPin()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errInvalidPin"));
        Outgoing::printLine();
    }

    void reportError_CmdTooLong(int length)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errCmdTooLong: "));
        Outgoing::printOutputStringMem(length);
        Outgoing::printLine();
    }

    void reportError_TooManyParams(int length)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errTooManyParams "));
        Outgoing::printOutputStringMem(length);
        Outgoing::printLine();
    }

    void reportError_MissingLibrary(const char *libName)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errMissingLibrary"));
        Outgoing::printOutputStringMem(libName);
        Outgoing::printLine();
    }

    void reportError_MultiMessageTimeout()
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errMultiMessageTimeout"));
        Outgoing::printLine();
    }

#ifdef RELAY_SUPPORTED
    void reportError_NoRelayForID(int id)
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errNoRelay: "));
        Outgoing::printOutputStringMem(id);
        Outgoing::printLine();
    }

    void reportError_RelayCollision(byte mac[6])
    {
        Outgoing::printLine();
        Outgoing::printOutputStringFlash(F("errRelayCollision: "));
        char buffer[20];
        UDIDHelper::convertMACToCStr(mac, buffer);
        Outgoing::printOutputStringMem(buffer);
        Outgoing::printLine();
    }
#endif

} // namespace Error
