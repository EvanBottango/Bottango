#include "AbstractMultiMessageOutgoingSource.h"
#include "System/Time.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"

void AbstractMultiMessageOutgoingSource::setRecievedContinue()
{
    hasOutgoingMessage = false;
    lastMessageTime = 0;
}

bool AbstractMultiMessageOutgoingSource::isTimeout()
{
    if (hasOutgoingMessage && Time::getCurrentTimeInMs() - lastMessageTime >= OUTGOING_TIMEOUT_RESPONSE_TIME)
    {
        return true;
    }
    return false;
}

void AbstractMultiMessageOutgoingSource::setTransmitted()
{
    hasOutgoingMessage = true;
    lastMessageTime = Time::getCurrentTimeInMs();
}

#ifdef RELAY_SUPPORTED
void AbstractMultiMessageOutgoingSource::setSecondary()
{
    secondary = true;
}
#endif