#ifndef AbstractMultiMessageOutgoingSource_h
#define AbstractMultiMessageOutgoingSource_h

#include <Arduino.h>
#include "../BottangoArduinoModules.h"

class AbstractMultiMessageOutgoingSource
{
public:
	// Multi-message flow:
	// 1) initializeMultiMessage() resets state, calls onMultiMessageStart(), and marks a pending emit.
	// 2) After the device sends OK, emitPending() sends the next chunk (emitNextChunk()).
	// 3) If emitNextChunk() returns true, we wait for setRecievedContinue() (host OK).
	// 4) setRecievedContinue() marks a pending emit for the next OK window.
	// 5) If emitNextChunk() returns false, the response ends (no further chunks).
	// 6) updateMultiMessage() only checks for timeout while waiting; timeout ends the response.

	// caller received a "continue" from the requester (ex: OK)
	void setRecievedContinue();

	// start a new multi-message response, next chunk is emitted after device OK is sent
	void initializeMultiMessage();

	// true when no more chunks are expected
	bool multiMessageisComplete() const;

	// handles timeouts (error state), emits next chunk when ready
	void updateMultiMessage();

	// emit next chunk if a continue is pending (used after device OK is sent)
	void emitPending();

	virtual void cleanUpMultiMessage() = 0; // cleanup if aborting...

protected:
	unsigned long _lastMessageTime = 0;
	bool _hasOutgoingMessage = false;

	// initialize responder state (iterator, flags, etc.)
	virtual void onMultiMessageStart() {};
	// emit at most one response chunk; base class drives timing
	virtual bool emitNextChunk() = 0;
	// called when a continue timeout is hit (error / abort)
	virtual void onTimeout();

	bool isTimeout() const;
	void setTransmitted();
	bool hasEmittedAny() const { return _emittedAny; }

private:
	bool _complete = false;
	bool _emittedAny = false;
	bool _pendingEmit = false;

	void tryEmitNextChunk();
};

#endif