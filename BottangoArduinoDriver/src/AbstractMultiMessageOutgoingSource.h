#ifndef AbstractMultiMessageOutgoingSource_h
#define AbstractMultiMessageOutgoingSource_h

#include <Arduino.h>
#include "../BottangoArduinoModules.h"
#include "Modules/Outgoing.h"

/**
 * @brief A base class for multi-message responses. Manages timing and flow control, derived classes implement chunk generation.
 */
class AbstractMultiMessageOutgoingSource
{
public:
	/**
	 * @par Multi-message flow:
	 * -# initializeMultiMessage() resets state, calls onMultiMessageStart(), and marks a pending emit.
	 * -# After the device sends OK, emitPending() sends the next chunk (emitNextChunk()).
	 * -# If emitNextChunk() returns true, we wait for setRecievedContinue() (host OK).
	 * -# setRecievedContinue() marks a pending emit for the next OK window.
	 * -# If emitNextChunk() returns false, the response ends (no further chunks).
	 * -# updateMultiMessage() only checks for timeout while waiting; timeout ends the response.
	 */

	// caller received a "continue" from the requester (ex: OK)
	void setRecievedContinue();

	// start a new multi-message response, next chunk is emitted after device OK is sent
	void initializeMultiMessage(OutgoingBase* outgoing);

	// true when no more chunks are expected
	bool multiMessageisComplete() const;

	// handles timeouts (error state), emits next chunk when ready
	void updateMultiMessage();

	// emit next chunk if a continue is pending (used after device OK is sent)
	void emitPending();

	virtual void cleanUpMultiMessage() = 0; // cleanup if aborting...

	void setOutgoing(OutgoingBase* outgoing) { _outgoing = outgoing; }

protected:
	unsigned long _lastMessageTime = 0;
	bool _hasOutgoingMessage = false;
	OutgoingBase* _outgoing = Outgoing::get();

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