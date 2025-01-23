#include "CommandStream.h"
#include "Arduino.h"
#include "../BottangoArduinoConfig.h"
#include "Log.h"
#include "Time.h"
#include "BottangoCore.h"

CommandStream::CommandStream(AbstractCommandStreamDataSource *dataSource)
{
    this->dataSource = dataSource;
}

void CommandStream::getNextCommand(char *output)
{
    unsigned long endTime = 0;
    dataSource->getNextCommand(output, shouldLoop, endTime, timeOfNextCommand);
    if (endTime > msEndOfLatestCommand)
    {
        msEndOfLatestCommand = endTime;
    }
}

bool CommandStream::readyForNextCommand()
{
#if defined(USE_SD_CARD_COMMAND_STREAM)
    dataSource->update(shouldLoop);
#endif

    // at end of loop
    if (shouldLoop && dataSource->dataComplete && Time::getCurrentTimeInMs() >= msEndOfLatestCommand)
    {
        // reset to beginning
        dataSource->reset();
        timeOfNextCommand = 0;
        msEndOfLatestCommand = 0;
        Time::syncTime(0);
        return true;
    }
    // at end of animation and not looping
    if (dataSource->dataComplete)
    {
        return false;
    }
#if defined(USE_SD_CARD_COMMAND_STREAM)
    // otherwise if at time of next command
    if (timeOfNextCommand > 10)
    {
        return Time::getCurrentTimeInMs() >= timeOfNextCommand - SD_ANIM_PREREAD_MS;
    }
    else if (timeOfNextCommand > 0)
    {
        return true;
    }
    else
    {
        return Time::getCurrentTimeInMs() >= timeOfNextCommand;
    }
#elif defined(USE_CODE_COMMAND_STREAM)
    return Time::getCurrentTimeInMs() >= timeOfNextCommand;
#endif

    return false;
}

bool CommandStream::complete()
{
    // looping is never complete, needs to be canceled externally
    if (shouldLoop)
    {
        return false;
    }

    return dataSource->dataComplete && Time::getCurrentTimeInMs() >= msEndOfLatestCommand;
}

void CommandStream::setShouldLoop()
{
    shouldLoop = true;
}

CommandStream::~CommandStream()
{
    delete dataSource;
}