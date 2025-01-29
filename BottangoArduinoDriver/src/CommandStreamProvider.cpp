#include "CommandStreamProvider.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"
#include "Outgoing.h"

#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"
#endif

CommandStreamProvider::CommandStreamProvider()
{
}

void CommandStreamProvider::runSetup()
{
    if (streamIsInProgress())
    {
        stop();
    }

#ifdef USE_CODE_COMMAND_STREAM
    commandStream = GeneratedCodeAnimations::GenerateSetupCommandStream();
#elif defined(USE_SD_CARD_COMMAND_STREAM)
    SDCardCommandStreamDataSource *dataSource = new SDCardCommandStreamDataSource();
    if (dataSource->isValid)
    {
        commandStream = new CommandStream(dataSource);
    }
    else
    {
        delete dataSource;
        commandStream = nullptr;
    }

#endif

    // run setup

#ifdef EXPORTED_ANIM_LOGGING
    Outgoing::printOutputStringFlash(F("Start setup"));
    Outgoing::printLine();
#endif

    runInProgressCommand();

    // parse configs
    // will also run start animation if any
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
    ExportedAnimationPlaybackControl::initialize();
#endif
}

void CommandStreamProvider::startCommandStream(byte streamID, bool loop)
{
#ifdef EXPORTED_ANIM_LOGGING
    Outgoing::printOutputStringFlash(F("Start anim: "));
    Outgoing::printOutputStringMem(streamID);
    Outgoing::printLine();
#endif

    if (streamIsInProgress())
    {
        stop();
    }

#ifdef ENABLE_STATUS_LIGHTS
    StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEPLAY);
#endif

#ifdef USE_CODE_COMMAND_STREAM
    commandStream = GeneratedCodeAnimations::GenerateCommandStreamByIndex(streamID);
#elif defined(USE_SD_CARD_COMMAND_STREAM)
    commandStream = new CommandStream(new SDCardCommandStreamDataSource(streamID, loop));
#endif

    if (commandStream != nullptr)
    {
        if (loop)
        {
            commandStream->setShouldLoop();
        }
        Time::syncTime(0);
    }
    else
    {
#ifdef EXPORTED_ANIM_LOGGING
        Outgoing::printOutputStringFlash(F("error starting anim"));
        Outgoing::printLine();
#endif
    }
}

void CommandStreamProvider::runInProgressCommand()
{
    if (streamIsInProgress())
    {
        while (commandStream->readyForNextCommand())
        {
            char commandBuffer[MAX_COMMAND_LENGTH];
            commandBuffer[0] = '\0';

            commandStream->getNextCommand(commandBuffer);
            if (commandBuffer[0] != '\0')
            {
#ifdef EXPORTED_ANIM_LOGGING
                Outgoing::printOutputStringMem(commandBuffer);
                Outgoing::printLine();
#endif
                BottangoCore::executeCommand(commandBuffer);
            }
        }

        if (commandStream->complete())
        {
#ifdef EXPORTED_ANIM_LOGGING
            Outgoing::printOutputStringFlash(F("stream complete"));
            Outgoing::printLine();
#endif
            stop();
        }
    }
}

void CommandStreamProvider::updateOnLoop()
{
    runInProgressCommand();
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
    ExportedAnimationPlaybackControl::updatePlaybackStatus();
#endif
}

void CommandStreamProvider::stop()
{
    delete commandStream;
    commandStream = nullptr;
    BottangoCore::effectorPool.clearAllCurves();

#ifdef ENABLE_STATUS_LIGHTS
    StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEREADY);
#endif
}

bool CommandStreamProvider::streamIsInProgress()
{
    return commandStream != nullptr;
}