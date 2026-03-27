#include "AbstractMultiMessageOutgoingSource.h"
#include "System/Time.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"

void AbstractMultiMessageOutgoingSource::setRecievedContinue()
{
    hasOutgoingMessage = false;
    _lastMessageTime = 0;
}

bool AbstractMultiMessageOutgoingSource::isTimeout()
{
    if (hasOutgoingMessage && Time::getCurrentTimeInMs() - _lastMessageTime >= OUTGOING_TIMEOUT_RESPONSE_TIME)
    {
        return true;
    }
    return false;
}

void AbstractMultiMessageOutgoingSource::setTransmitted()
{
    hasOutgoingMessage = true;
    _lastMessageTime = Time::getCurrentTimeInMs();
}

#ifdef RELAY_SUPPORTED
void AbstractMultiMessageOutgoingSource::setSecondary()
{
    secondary = true;
}
#endif