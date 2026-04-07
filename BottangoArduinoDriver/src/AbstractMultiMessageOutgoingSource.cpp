#include "AbstractMultiMessageOutgoingSource.h"
#include "System/Time.h"
#include "../BottangoArduinoConfig.h"
#include "Errors.h"

void AbstractMultiMessageOutgoingSource::initializeMultiMessage(OutgoingBase* outgoing)
{
	_hasOutgoingMessage = false;
	_lastMessageTime = 0;
	_complete = false;
	_emittedAny = false;
	_pendingEmit = true;
	_outgoing = outgoing;
	onMultiMessageStart();
}

void AbstractMultiMessageOutgoingSource::setRecievedContinue()
{
	_hasOutgoingMessage = false;
	_lastMessageTime = 0;
	_pendingEmit = true;
}

bool AbstractMultiMessageOutgoingSource::multiMessageisComplete() const
{
	return _complete;
}

void AbstractMultiMessageOutgoingSource::updateMultiMessage()
{
	if (_hasOutgoingMessage)
	{
		if (isTimeout())
		{
			onTimeout();
			_hasOutgoingMessage = false;
			_lastMessageTime = 0;
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
	if (_complete || _hasOutgoingMessage)
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

bool AbstractMultiMessageOutgoingSource::isTimeout() const
{
	if (_hasOutgoingMessage && Time::getCurrentTimeInMs() - _lastMessageTime >= OUTGOING_TIMEOUT_RESPONSE_TIME)
	{
		return true;
	}
	return false;
}

void AbstractMultiMessageOutgoingSource::setTransmitted()
{
	_hasOutgoingMessage = true;
	_emittedAny = true;
	_lastMessageTime = Time::getCurrentTimeInMs();
}

void AbstractMultiMessageOutgoingSource::emitPending()
{
	if (_complete || _hasOutgoingMessage)
	{
		return;
	}

	if (_pendingEmit)
	{
		tryEmitNextChunk();
	}
}