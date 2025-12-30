#ifndef TXTBUFFER_H
#define TXTBUFFER_H

#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW) || defined(USE_SD_CARD_COMMAND_STREAM)

#include "../BottangoArduinoConfig.h"
#include <cstring>
#include "Outgoing.h"

#ifdef ESP32
#include <freertos/semphr.h>
#endif

template <int BufferSize>
class TxtBuffer
{
public:
    TxtBuffer()
    {
        head = 0;
        tail = 0;
        buffer[0] = '\0';

#ifdef ESP32
        mutex = xSemaphoreCreateMutex();
        configASSERT(mutex);
#endif
    }

    void addTxt(const char *txt)
    {
        int len = strlen(txt);
        if (len > MAX_COMMAND_LENGTH)
            len = MAX_COMMAND_LENGTH;

#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif

        for (int i = 0; i < len; ++i)
        {
            if (isFullInternal())
            {
                break;
            }
            buffer[head] = txt[i];
            head = (head + 1) % BufferSize;

            // Handle overflow by advancing tail if head catches up
            if (head == tail)
            {
                tail = (tail + 1) % BufferSize;
            }
        }
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
    }

    void addChar(const char c)
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        if (!isFullInternal())
        {
            buffer[head] = c;
            head = (head + 1) % BufferSize;
            if (head == tail)
            {
                tail = (tail + 1) % BufferSize;
            }
        }
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
    }

    bool available()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        bool has = (head != tail);
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
        return has;
    }

    int getSpaceAvailable()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        int space;
        if (head >= tail)
        {
            space = BufferSize - (head - tail) - 1;
        }
        else
        {
            space = (tail - head) - 1;
        }
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
        return space;
    }

    int getSpaceUsed()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        int used;
        if (head >= tail)
        {
            used = head - tail;
        }
        else
        {
            used = BufferSize - (tail - head);
        }
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
        return used;
    }

    // Pop and return one full line ending in '\n'.
    // If no complete line, outTxt = "" and buffer untouched.
    // If peek==true, buffer is not advanced after successful read.
    void getNextTxt(char *outTxt, bool peek = false)
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        // No data
        if (head == tail)
        {
            outTxt[0] = '\0';
#ifdef ESP32
            xSemaphoreGive(mutex);
#endif
            return;
        }

        // Scan up to MAX_COMMAND_LENGTH for a newline
        int tempTail = tail;
        char temp[MAX_COMMAND_LENGTH];
        int len = 0;
        bool foundNewline = false;
        while (tempTail != head && len < MAX_COMMAND_LENGTH - 1)
        {
            char c = buffer[tempTail];
            temp[len++] = c;
            tempTail = (tempTail + 1) % BufferSize;
            if (c == '\n')
            {
                foundNewline = true;
                break;
            }
        }

        if (!foundNewline)
        {
            // No complete line, return empty
            outTxt[0] = '\0';
        }
        else
        {
            // Copy out the full line
            memcpy(outTxt, temp, len);
            outTxt[len] = '\0';
            if (!peek)
            {
                // Advance tail past the line
                tail = tempTail;
            }
        }

#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
    }

    char getNextChar()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        char c = '\0';
        if (head != tail)
        {
            c = buffer[tail];
            tail = (tail + 1) % BufferSize;
        }
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
        return c;
    }

    bool isFull()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        bool full = isFullInternal();
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
        return full;
    }

    // Clear entire buffer
    void clear()
    {
#ifdef ESP32
        xSemaphoreTake(mutex, portMAX_DELAY);
#endif
        head = 0;
        tail = 0;
        buffer[0] = '\0';
#ifdef ESP32
        xSemaphoreGive(mutex);
#endif
    }

private:
    // Internal check without taking mutex
    bool isFullInternal() const
    {
        return ((head + 1) % BufferSize) == tail;
    }

    char buffer[BufferSize];
    int head;
    int tail;
#ifdef ESP32
    SemaphoreHandle_t mutex;
#endif
};

#endif // RELAY_COMS_* || USE_SD_CARD_COMMAND_STREAM
#endif // TXTBUFFER_H
