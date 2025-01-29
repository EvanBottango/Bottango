#include "../BottangoArduinoModules.h"

#ifdef USE_SD_CARD_COMMAND_STREAM
#include "SDCardCommandStreamDataSource.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"

#define THRESHOLD_TO_READ_MORE ((MAX_COMMAND_LENGTH * 2) + 2)

SDCardCommandStreamDataSource::SDCardCommandStreamDataSource() : AbstractCommandStreamDataSource()
{
    SDCardUtil::initialize();
    if (SDCardUtil::sdCardInitialized && SDCardUtil::sdCardAvailable)
    {
        this->setup = true;
        isValid = true;
    }
    dataComplete = false;
}

SDCardCommandStreamDataSource::SDCardCommandStreamDataSource(byte index, bool shouldLoop) : AbstractCommandStreamDataSource()
{
    SDCardUtil::initialize();
    if (SDCardUtil::sdCardInitialized && SDCardUtil::sdCardAvailable)
    {
        this->index = index;
        this->setup = false;
        isValid = true;
    }
    dataComplete = false;
    cardReadComplete = false;
    update(shouldLoop);
}

void SDCardCommandStreamDataSource::update(bool shouldLoop)
{
    // Check if more data should be loaded into the buffer
    if (!cardReadComplete && cardReadBuffer.getSpaceUsed() <= THRESHOLD_TO_READ_MORE)
    {

        if (!currentFile)
        {
            // open the right file
            char finalPath[MAX_FILE_PATH_SIZE];
            if (setup)
            {
                SDCardUtil::getSetupFilePath(finalPath);
            }
            else
            {
                SDCardUtil::getAnimationFilePath(index, finalPath, onLoop, false);
            }

            currentFile = SDCardUtil::openFile(finalPath);

            // couldn't open file
            if (!currentFile)
            {
                cardReadComplete = true;
                return;
            }
        }

        char tempBuffer[TXT_BUFFER_READ_CHUNK_SIZE];
        // Keep reading blocks until the buffer is nearly full
        while (currentFile.available() && cardReadBuffer.getSpaceAvailable() >= TXT_BUFFER_READ_CHUNK_SIZE)
        {
            int bytesRead = currentFile.readBytes(tempBuffer, TXT_BUFFER_READ_CHUNK_SIZE);

            for (int i = 0; i < bytesRead; i++)
            {
                cardReadBuffer.addChar(tempBuffer[i]);
            }

            if (!currentFile.available())
            {
                // should we loop?
                if (shouldLoop)
                {
                    // move to loop
                    if (!onLoop)
                    {
                        onLoop = true;
                        char finalPath[MAX_FILE_PATH_SIZE];
                        SDCardUtil::getAnimationFilePath(index, finalPath, true, false);
                        SDCardUtil::closeFile(currentFile);

                        // end if can't find loop file
                        currentFile = SDCardUtil::openFile(finalPath);
                        if (!currentFile)
                        {
                            cardReadComplete = true;
                            SDCardUtil::closeFile(currentFile);
                            return;
                        }
                    }
                    // loop complete
                    else
                    {
                        cardReadComplete = true;
                        SDCardUtil::closeFile(currentFile);
                        return;
                    }
                }
                // non loop complete
                else
                {
                    cardReadComplete = true;
                    SDCardUtil::closeFile(currentFile);
                    return;
                }
            }
        }
    }
}

void SDCardCommandStreamDataSource::getNextCommand(char *output, bool shouldLoop, unsigned long &msEndOfThisCommand, unsigned long &msStartOfNextCommand)
{
    if (!SDCardUtil::sdCardInitialized || !SDCardUtil::sdCardAvailable)
    {
        return;
    }
    update(shouldLoop);

    cardReadBuffer.getNextTxt(output);

    // get output
    if (output[0] != '\0')
    {

        // copy as we're gonna split it to find the end time
        char copy[strlen(output) + 1]; // +1 for the null terminator
        strcpy(copy, output);
        msEndOfThisCommand = BottangoCore::getMSEndTimeOfCommand(copy);
    }
    else
    {
        dataComplete = true;
    }

    // find next command
    update(shouldLoop);
    char nextCommand[MAX_COMMAND_LENGTH];
    nextCommand[0] = '\0';
    cardReadBuffer.getNextTxt(nextCommand, true);
    if (nextCommand[0] != '\0')
    {
        msStartOfNextCommand = BottangoCore::getMSStartTimeOfCommand(nextCommand);
    }
}

void SDCardCommandStreamDataSource::reset()
{
    onLoop = false;
    dataComplete = false;
    cardReadComplete = false;
    cardReadBuffer.clear();
}

SDCardCommandStreamDataSource::~SDCardCommandStreamDataSource()
{
}
#endif