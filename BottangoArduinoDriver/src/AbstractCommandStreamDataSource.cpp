#include "AbstractMultiMessageOutgoingSource.h"
#include "System/Time.h"
#include "../BottangoArduinoConfig.h"
#include "Errors.h"

void AbstractMultiMessageOutgoingSource::initializeMultiMessage()
{
	hasOutgoingMessage = false;
	lastMessageTime = 0;
	_complete = false;
	_emittedAny = false;
	_pendingEmit = true;
	onMultiMessageStart();
}

void AbstractMultiMessageOutgoingSource::setRecievedContinue()
{
	hasOutgoingMessage = false;
	lastMessageTime = 0;
	_pendingEmit = true;
}

bool AbstractMultiMessageOutgoingSource::multiMessageisComplete()
{
	return _complete;
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
			_complete = true;
		}
		return;
	}

	if (!_complete && _pendingEmit)
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
	if (_complete || hasOutgoingMessage)
	{
		return;
	}

	_pendingEmit = false;
	if (emitNextChunk())
	{
		setTransmitted();
	}
	else
	{
		_complete = true;
	}
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
	_emittedAny = true;
	lastMessageTime = Time::getCurrentTimeInMs();
}

void AbstractMultiMessageOutgoingSource::emitPending()
{
	if (_complete || hasOutgoingMessage)
	{
		return;
	}

	if (_pendingEmit)
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