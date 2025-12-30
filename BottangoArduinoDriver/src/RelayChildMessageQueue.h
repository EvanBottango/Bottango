#ifndef RELAY_CHILD_MESSAGE_QUEUE_H
#define RELAY_CHILD_MESSAGE_QUEUE_H

#include "../BottangoArduinoModules.h"

#ifdef RELAY_SUPPORTED

#include "../BottangoArduinoConfig.h"
#include <freertos/queue.h>
#include <cstring>

#ifdef TOGGLE_DEBUG
#include "PersistentConfigUtil.h"
#endif

static constexpr int MAX_PAYLOAD = MAX_COMMAND_LENGTH;

struct OutgoingMessage
{
    uint8_t mac[6];
    uint16_t length;
    char payload[MAX_PAYLOAD];
};

class RelayChildMessageQueue
{
public:
    RelayChildMessageQueue()
    {
        queue = xQueueCreate(OUT_MESSAGE_QUEUE_DEPTH, sizeof(OutgoingMessage));
        configASSERT(queue);
    }

    bool enqueue(const uint8_t mac[6], const char *txt)
    {
        OutgoingMessage msg;

        memcpy(msg.mac, mac, 6);

        msg.length = strnlen(txt, MAX_PAYLOAD);

        memcpy(msg.payload, txt, msg.length);

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled())
#endif
        {
            Outgoing::printOutputStringFlash(F("To Queue "));
            Outgoing::printOutputStringMem(txt);
            Outgoing::printLine;
        }
#endif

        return xQueueSend(queue, &msg, 0) == pdTRUE;
    }

    bool dequeue(OutgoingMessage &out)
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

    bool peek(OutgoingMessage &out) const
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

#endif // relay supported
#endif // OUTGOING_MESSAGE_QUEUE_H