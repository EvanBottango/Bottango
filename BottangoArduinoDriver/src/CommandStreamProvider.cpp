#include "CommandStreamProvider.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"

#ifdef USE_COMMAND_STREAM
#include "../GeneratedCommandStreams.h"
#endif

CommandStreamProvider::CommandStreamProvider()
{
}

void CommandStreamProvider::runSetup()
{
    LOG(F("start setup"));

    CommandStream *commandStream = NULL;

#ifdef USE_COMMAND_STREAM
    commandStream = GeneratedCommandStreams::getSetupCommandStream();
#endif

    if (commandStream != NULL)
    {
        if (streamIsInProgress())
        {
            stop();
        }
        streamInProgress = commandStream;
        streamInProgress->reset();
        runInProgressCommand();
    }
}

void CommandStreamProvider::startCommandStream(byte streamID, bool loop)
{
    LOG(F("start baked: "));
    LOG_LN(streamID);

    CommandStream *commandStream = NULL;

#ifdef USE_COMMAND_STREAM
    commandStream = GeneratedCommandStreams::getCommandStream(streamID);
#endif

    if (commandStream != NULL)
    {
        if (streamIsInProgress())
        {
            stop();
        }
        streamInProgress = commandStream;
        if (loop)
        {
            streamInProgress->setShouldLoop();
        }
        Time::syncTime(0);
    }
}

void CommandStreamProvider::runInProgressCommand()
{
    if (streamIsInProgress())
    {
        while (streamInProgress->readyForNextCommand())
        {
            char commandBuffer[MAX_COMMAND_LENGTH];
            commandBuffer[0] = '\0';

            streamInProgress->getNextCommand(commandBuffer);

            if (strlen(commandBuffer) > 0)
            {
                LOG(F("baked command: "));
                LOG_LN(commandBuffer);
            }

            BottangoCore::commandRegistry.executeCommand(commandBuffer);
        }

        if (streamInProgress->complete())
        {
            stop();
        }
    }
}

void CommandStreamProvider::updateOnLoop()
{
    runInProgressCommand();

#ifdef USE_COMMAND_STREAM
    GeneratedCommandStreams::updatePlayStatus();
#endif
}

void CommandStreamProvider::stop()
{
    if (streamIsInProgress())
    {
        streamInProgress->reset();
    }
    streamInProgress = NULL;
    BottangoCore::effectorPool.clearAllCurves();
}

bool CommandStreamProvider::streamIsInProgress()
{
    return streamInProgress != NULL;
}