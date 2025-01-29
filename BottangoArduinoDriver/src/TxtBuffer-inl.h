#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW) || defined(USE_SD_CARD_COMMAND_STREAM)

#include "TxtBuffer.h"

template <int BufferSize>
TxtBuffer<BufferSize>::TxtBuffer()
{
    head = 0;
    tail = 0;
    buffer[0] = '\0';
}

template <int BufferSize>
void TxtBuffer<BufferSize>::addTxt(const char *txt)
{
    int len = strlen(txt);
    if (len > MAX_COMMAND_LENGTH)
    {
        // todo error
        len = MAX_COMMAND_LENGTH;
    }

    for (int i = 0; i < len; i++)
    {
        // Check if the buffer is full before adding more characters
        if (isFull())
        {
            // todo error
            break; // Stop adding text if the buffer is full
        }

        buffer[head] = txt[i];
        head = (head + 1) % BufferSize;

        // Handle buffer overflow
        if (head == tail)
        {
            tail = (tail + 1) % BufferSize;
        }
    }
}

template <int BufferSize>
void TxtBuffer<BufferSize>::addChar(const char c)
{
    // Check if the buffer is full before adding more characters
    if (isFull())
    {
        // todo error
        return; // Stop adding text if the buffer is full
    }

    buffer[head] = c;
    head = (head + 1) % BufferSize;

    // Handle buffer overflow
    if (head == tail)
    {
        tail = (tail + 1) % BufferSize;
    }
}

template <int BufferSize>
bool TxtBuffer<BufferSize>::available()
{
    return head != tail;
}

template <int BufferSize>
void TxtBuffer<BufferSize>::getNextTxt(char *outTxt, bool peek)
{
    if (!available())
    {
        outTxt[0] = '\0'; // Return empty string if no data
        return;
    }

    int i = 0;

    int tailCache = tail;
    int headCache = head;

    while (tail != head && i < MAX_COMMAND_LENGTH - 1)
    {
        outTxt[i] = buffer[tail];
        char currentChar = buffer[tail];
        tail = (tail + 1) % BufferSize;
        i++;

        if (currentChar == '\n')
        {
            break; // Stop at newline but include it
        }
    }

    outTxt[i] = '\0'; // null terminate the output

    if (peek)
    {
        tail = tailCache;
        head = headCache;
    }
}

template <int BufferSize>
bool TxtBuffer<BufferSize>::isFull()
{
    return ((head + 1) % BufferSize) == tail;
}

template <int BufferSize>
void TxtBuffer<BufferSize>::clear()
{
    head = 0;
    tail = 0;
    buffer[0] = '\0';
}

template <int BufferSize>
int TxtBuffer<BufferSize>::getSpaceAvailable()
{
    if (head >= tail)
    {
        return BufferSize - (head - tail) - 1;
    }
    else
    {
        return (tail - head) - 1;
    }
}

template <int BufferSize>
int TxtBuffer<BufferSize>::getSpaceUsed()
{
    if (head >= tail)
    {
        return head - tail;
    }
    else
    {
        return BufferSize - (tail - head);
    }
}
#endif