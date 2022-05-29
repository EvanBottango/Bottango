#include "CommandStream.h"
#include "Arduino.h"
#include "../BottangoArduinoConfig.h"
#include "Log.h"
#include "Time.h"
#include "BottangoCore.h"

CommandStream::CommandStream(const char *charStream, unsigned long streamDurationInMS)
{
    this->charStream = charStream;
    this->streamDurationInMS = streamDurationInMS;

    this->loopCharStream = NULL;
    this->loopStreamDurationInMS = 0;

    streamLength = strlen_P(this->charStream);
    loopStreamLength = 0;
}

CommandStream::CommandStream(const char *charStream, unsigned long streamDurationInMS, const char *loopCharStream, unsigned long loopStreamDurationInMS)
{
    this->charStream = charStream;
    this->streamDurationInMS = streamDurationInMS;

    this->loopCharStream = loopCharStream;
    this->loopStreamDurationInMS = loopStreamDurationInMS;

    streamLength = strlen_P(this->charStream);
    loopStreamLength = strlen_P(this->loopCharStream);
}

void CommandStream::getNextCommand(char *output)
{
    int outputIterator = 0;

    while ((!onLoop && travel < streamLength) || (onLoop && travel < loopStreamLength))
    {
        char nextChar;
        if (onLoop)
        {
            nextChar = (char)pgm_read_byte(loopCharStream + travel);
        }
        else
        {
            nextChar = (char)pgm_read_byte(charStream + travel);
        }

        travel++;

        if (nextChar == '\n' || outputIterator >= MAX_COMMAND_LENGTH)
        {
            output[outputIterator] = '\0';

            if (!onLoop && shouldLoop && travel >= streamLength)
            {
                onLoop = true;
                travel = 0;
            }
            return;
        }

        output[outputIterator] = nextChar;
        outputIterator++;
    }
}

bool CommandStream::readyForNextCommand()
{
    if ((!onLoop && travel >= streamLength) || (onLoop && travel >= loopStreamLength))
    {
        return false;
    }

    char buffer[MAX_COMMAND_LENGTH];
    unsigned int bufferIterator = 0;
    unsigned long checkIterator = travel;

    while ((!onLoop && checkIterator < streamLength) || (onLoop && checkIterator < loopStreamLength))
    {
        char nextChar;
        if (onLoop)
        {
            nextChar = (char)pgm_read_byte(loopCharStream + checkIterator);
        }
        else
        {
            nextChar = (char)pgm_read_byte(charStream + checkIterator);
        }

        checkIterator++;
        if (nextChar == '\n' || bufferIterator >= MAX_COMMAND_LENGTH)
        {
            buffer[bufferIterator] = '\0';
            unsigned long commandTime = BottangoCore::commandRegistry.getMSTimeOfCommand(buffer);

            return commandTime <= Time::getCurrentTimeInMs();
        }

        buffer[bufferIterator] = nextChar;
        bufferIterator++;
    }
    return false;
}

bool CommandStream::complete()
{
    if (shouldLoop)
    {
        if (onLoop)
        {
            return (travel >= loopStreamLength && Time::getCurrentTimeInMs() >= streamDurationInMS + loopStreamDurationInMS);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return (travel >= streamLength && Time::getCurrentTimeInMs() >= streamDurationInMS);
    }
}

void CommandStream::reset()
{
    travel = 0;
    shouldLoop = false;
    onLoop = false;
}

void CommandStream::setShouldLoop()
{
    if (loopStreamLength == 0)
    {
        return;
    }
    shouldLoop = true;
}