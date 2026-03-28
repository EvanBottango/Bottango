#include "MACResponder.h"
#if defined(RELAY_SUPPORTED) && defined(RELAY_COMS_ESPNOW)
#include "PersistentConfigUtil.h"
#include "Outgoing.h"

void MACResponder::initializeMultiMessage()
{
    macSent = false;
}

bool MACResponder::multiMessageisComplete()
{
    return macSent && !_hasOutgoingMessage;
}

void MACResponder::updateMultiMessage()
{
    if (!macSent)
    {
        char buffer[15];
        PersistentConfigUtil::getThisDeviceMacAddress(buffer);
#ifdef RELAY_SUPPORTED
        if (_secondary)
        {
            Outgoing::setSecondaryPeerOutgoing(true);
        }
#endif
        Outgoing::printOutputStringPROGMEM(REPLY_MAC_ADDRESS);
        Outgoing::printOutputStringFlash(F(","));
        Outgoing::printOutputStringMem(buffer);
        Outgoing::printLine();
#ifdef RELAY_SUPPORTED
        if (_secondary)
        {
            Outgoing::setSecondaryPeerOutgoing(false);
        }
#endif

        macSent = true;
    }
}

void MACResponder::cleanUpMultiMessage()
{
    delete this;
}

#endif