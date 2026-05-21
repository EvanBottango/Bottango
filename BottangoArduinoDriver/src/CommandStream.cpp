#include "CommandStream.h"
#include "Arduino.h"
#include "../BottangoArduinoConfig.h"
#include "Time.h"
#include "BottangoCore.h"

CommandStream::CommandStream(AbstractCommandStreamDataSource *dataSource)
{
    this->dataSource = dataSource;
}

void CommandStream::getNextCommand(char *output)
{
    dataSource->getNextCommand(output, shouldLoop, false);

#ifndef ESP32
    // got something empty back? Let's update data source and try one more time
    if (output[0] == '\0')
    {
        dataSource->updateOnLoop();
        dataSource->getNextCommand(output, shouldLoop, false);
    }
#endif

    if (output[0] != '\0')
    {
        char outputCopy[MAX_COMMAND_LENGTH] = {0};
        strcpy(outputCopy, output);
        long endTime = BottangoCore::getMSTimeOfCommand(outputCopy, false);
        if (endTime > msEndOfLatestCommand)
        {
            msEndOfLatestCommand = endTime;
        }

        char nextCommandBuffer[MAX_COMMAND_LENGTH] = {0};
        dataSource->getNextCommand(nextCommandBuffer, shouldLoop, true);
        if (nextCommandBuffer[0] != '\0')
        {
            timeOfNextCommand = BottangoCore::getMSTimeOfCommand(nextCommandBuffer, true);
        }
    }
}

bool CommandStream::readyForNextCommand()
{
    // at end of loop
    if (shouldLoop && dataSource->dataComplete && Time::getCurrentTimeInMs() >= msEndOfLatestCommand)
    {
        // reset to beginning

        BottangoCore::effectorPool.clearAllCurves();

#ifdef RELAY_SUPPORTED
        if (BottangoCore::isRelayBridge)
        {
            // Reset curves and time at end of loop before reset
            BottangoCore::relayPool->clearCurvesOnConnectedPeers();
            BottangoCore::relayPool->resumeTimeConnectedPeers(true);
        }
#endif
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

#ifdef RELAY_SUPPORTED
    // not ready for next if esp comms pool is full
    if (BottangoCore::isRelayBridge && BottangoCore::relayPool->toPeerQueueFull())
    {
        return false;
    }

    // check pre read time?
    if (timeOfNextCommand > SD_ANIM_PREREAD_MS_RELAY)
    {
        return Time::getCurrentTimeInMs() >= timeOfNextCommand - SD_ANIM_PREREAD_MS;
    }
#else
    if (timeOfNextCommand > SD_ANIM_PREREAD_MS)
    {
        return Time::getCurrentTimeInMs() >= timeOfNextCommand - SD_ANIM_PREREAD_MS;
    }
#endif

    // otherwise all commands before pre-read are valid
    else if (timeOfNextCommand > 0)
    {
        return true;
    }
    // fallback if we're at or past time of next
    else
    {
        return Time::getCurrentTimeInMs() >= timeOfNextCommand;
    }
#elif defined(USE_CODE_COMMAND_STREAM)
#ifdef RELAY_SUPPORTED
    // not ready for next if esp comms pool is full
    if (BottangoCore::isRelayBridge && BottangoCore::relayPool->toPeerQueueFull())
    {
        return false;
    }
#endif
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

void CommandStream::updateOnLoop()
{
    if (dataSource != nullptr)
    {
        dataSource->updateOnLoop();
    }
}

CommandStream::~CommandStream()
{
    delete dataSource;
}