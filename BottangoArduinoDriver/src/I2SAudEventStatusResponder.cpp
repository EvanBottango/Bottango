#include "I2SAudEventStatusResponder.h"
#if defined(AUDIO_SD_I2S)

#include "Outgoing.h"
#include "SDCardUtil.h"
#include "../BottangoArduinoConfig.h"

I2SAudEventStatusResponder::I2SAudEventStatusResponder(byte incomingStatus)
{
    status = incomingStatus;
}

void I2SAudEventStatusResponder::initializeMultiMessage()
{
    statusSent = false;
}

bool I2SAudEventStatusResponder::multiMessageisComplete()
{
    return statusSent && !hasOutgoingMessage;
}

void I2SAudEventStatusResponder::updateMultiMessage()
{
    if (!statusSent)
    {
#ifdef RELAY_SUPPORTED
        if (secondary)
        {
            Outgoing::setSecondaryPeerOutgoing(true);
        }
#endif
        Outgoing::printOutputStringPROGMEM(REPLY_AUD_STATUS);
        Outgoing::printOutputStringFlash(F(","));
        Outgoing::printOutputStringMem(status);
        Outgoing::printLine();

#ifdef RELAY_SUPPORTED
        if (secondary)
        {
            Outgoing::setSecondaryPeerOutgoing(false);
        }
#endif

        statusSent = true;
    }
}

void I2SAudEventStatusResponder::cleanUpMultiMessage()
{
    delete this;
}

#endif