#ifndef RELAY_CHILD_MESSAGE_QUEUE_H
#define RELAY_CHILD_MESSAGE_QUEUE_H

#include "../BottangoArduinoModules.h"

#ifdef RELAY_SUPPORTED

#include "../BottangoArduinoConfig.h"
#include <freertos/queue.h>
#include <cstring>

#ifdef TOGGLE_DEBUG
#include "PersistentConfigUtil.h"
#endif // TOGGLE_DEBUG

static constexpr int MAX_PAYLOAD = MAX_COMMAND_LENGTH;

enum class MessageIntent : uint8_t
{
	Normal,
	Poll,
	Boot
};

enum class TargetGroup : uint8_t
{
	Unicast,
	BroadcastConnected,
	BroadcastUnconnected
};

struct OutgoingMessage
{
	int peerId;
	uint16_t length;
	MessageIntent intent;
	TargetGroup target;
	uint8_t payload[MAX_PAYLOAD];
};

class RelayChildMessageQueue
{
public:
	RelayChildMessageQueue()
	{
		queue = xQueueCreate(OUT_MESSAGE_QUEUE_DEPTH, sizeof(OutgoingMessage));
		configASSERT(queue);
	}

	bool enqueueMessage(const int peerId, const char* txt, MessageIntent intent, TargetGroup target)
	{
		OutgoingMessage msg;
		msg.peerId = peerId;
		msg.intent = intent;
		msg.target = target;

		msg.length = strnlen(txt, MAX_PAYLOAD);

		// Heads up! Raw payload buffer, not null-terminated. Relay comms must send by length.
		memcpy(msg.payload, txt, msg.length);

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			Outgoing::printOutputStringFlash(F("To Queue "));
			Outgoing::printOutputStringMem(txt);
			Outgoing::printLine;
		}
#endif // RELAY_LOGGING

		return xQueueSend(queue, &msg, 0) == pdTRUE;
	}

	bool dequeue(OutgoingMessage& out)
	{
		return xQueueReceive(queue, &out, 0) == pdTRUE;
	}

	bool empty() const
	{
		return uxQueueMessagesWaiting(queue) == 0;
	}

	bool full() const
	{
		return uxQueueSpacesAvailable(queue) == 0;
	}

	bool peek(OutgoingMessage& out) const
	{
		return xQueuePeek(queue, &out, 0) == pdTRUE;
	}

	void pop()
	{
		OutgoingMessage dummy;
		xQueueReceive(queue, &dummy, 0);
	}

private:
	QueueHandle_t queue;
};

#endif // RELAY_SUPPORTED
#endif // OUTGOING_MESSAGE_QUEUE_H