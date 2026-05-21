#include "AbstractMultiMessageOutgoingSource.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"
#include "Errors.h"

void AbstractMultiMessageOutgoingSource::initializeMultiMessage()
{
    hasOutgoingMessage = false;
    lastMessageTime = 0;
    complete = false;
    emittedAny = false;
    pendingEmit = true;
    onMultiMessageStart();
}

void AbstractMultiMessageOutgoingSource::setRecievedContinue()
{
    hasOutgoingMessage = false;
    lastMessageTime = 0;
    pendingEmit = true;
}

bool AbstractMultiMessageOutgoingSource::multiMessageisComplete()
{
    return complete;
}

void AbstractMultiMessageOutgoingSource::updateMultiMessage()
{
    if (hasOutgoingMessage)
    {
        if (isTimeout())
        {
            onTimeout();
            hasOutgoingMessage = false;
            lastMessageTime = 0;
            complete = true;
        }
        return;
    }

    if (!complete && pendingEmit)
    {
        tryEmitNextChunk();
    }
}

void AbstractMultiMessageOutgoingSource::onTimeout()
{
    Error::reportError_MultiMessageTimeout();
}

void AbstractMultiMessageOutgoingSource::tryEmitNextChunk()
{
    if (complete || hasOutgoingMessage)
    {
        return;
    }

    pendingEmit = false;
    if (emitNextChunk())
    {
        setTransmitted();
    }
    else
    {
        complete = true;
    }
}

bool AbstractMultiMessageOutgoingSource::isTimeout()
{
    if (hasOutgoingMessage && millis() - lastMessageTime >= OUTGOING_TIMEOUT_RESPONSE_TIME)
    {
        return true;
    }
    return false;
}

void AbstractMultiMessageOutgoingSource::setTransmitted()
{
    hasOutgoingMessage = true;
    emittedAny = true;
    lastMessageTime = millis();
}

void AbstractMultiMessageOutgoingSource::emitPending()
{
    if (complete || hasOutgoingMessage)
    {
        return;
    }

    if (pendingEmit)
    {
        tryEmitNextChunk();
    }
}

#ifdef RELAY_SUPPORTED
void AbstractMultiMessageOutgoingSource::setSecondary()
{
    secondary = true;
}
#endif
