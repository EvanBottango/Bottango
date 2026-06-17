#include "../BottangoArduinoModules.h"

#ifdef USE_SD_CARD_COMMAND_STREAM
#include "SDCardCommandStreamDataSource.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"

/*
 * SDCardCommandStreamDataSource: Streams animation commands from SD card files
 *
 * Architecture: Dual-mode operation based on platform capabilities
 * - ESP32: Background FreeRTOS task continuously fills circular buffer
 * - Other platforms: Main loop fills buffer during idle time via updateOnLoop()
 *
 * ESP32 Task Lifecycle:
 * 1. Created in constructor if files valid, runs fillTask() continuously
 * 2. Task reads chunks when buffer has space, blocks on ulTaskNotifyTake() when full
 * 3. getNextCommand() signals task via xTaskNotifyGive() when buffer space available
 * 4. Task auto-exits when cardReadComplete=true, cleans up file handles
 * 5. Destructor/reset gracefully terminates task with timeout + forced cleanup
 */

#define THRESHOLD_TO_READ_MORE ((MAX_COMMAND_LENGTH * 2) + 2)

// Setup-only constructor: reads board configuration, never loops
SDCardCommandStreamDataSource::SDCardCommandStreamDataSource()
    : AbstractCommandStreamDataSource()
{

    this->setup = true;
    this->shouldLoop = false;
    dataComplete = false;
    cardReadComplete = false;
    checkIsValid();
    if (isValid)
    {
#ifdef ESP32
        startFillTask(); // ESP32: Background task fills buffer
#else
        updateOnLoop(); // Non-ESP32: Buffer filled during main loop via updateOnLoop(), but needs to be primed
#endif
    }
}

// Animation constructor: reads indexed animation files, supports looping
SDCardCommandStreamDataSource::SDCardCommandStreamDataSource(byte index,
                                                             bool shouldLoop)
    : AbstractCommandStreamDataSource()
{

    this->index = index;
    this->setup = false;
    this->shouldLoop = shouldLoop;
    dataComplete = false;
    cardReadComplete = false;
    checkIsValid();
    if (isValid)
    {
#ifdef ESP32
        startFillTask(); // ESP32: Background task fills buffer
#else
        updateOnLoop(); // Non-ESP32: Buffer filled during main loop via updateOnLoop(), but needs to be primed
#endif
    }
}

SDCardCommandStreamDataSource::~SDCardCommandStreamDataSource()
{
    // Graceful task shutdown with timeout to prevent hangs

#ifdef ESP32
    cardReadComplete = true;
    if (fillTaskHandle)
    {
        // Notify the task to wake up and see the cardReadComplete flag
        xTaskNotifyGive(fillTaskHandle);

        // Wait for the task to clean itself up (with timeout)
        int timeout = 1000; // 1 second timeout
        while (fillTaskHandle != nullptr && timeout > 0)
        {
            vTaskDelay(1);
            timeout--;
        }

        // Force delete if task didn't exit gracefully
        if (fillTaskHandle != nullptr)
        {
            vTaskDelete(fillTaskHandle);
            fillTaskHandle = nullptr;
        }
    }
#endif
}

void SDCardCommandStreamDataSource::checkIsValid()
{
    // Validate required files exist before starting buffer operations
    char path[MAX_FILE_PATH_SIZE];
    path[0] = '\0';
    SDCardUtil::SDFileError fileError;

    if (setup)
    {
        SDCardUtil::getSetupFilePath(path);
    }
    else
    {
        SDCardUtil::getAnimationFilePath(index, path, false, false);
    }

    File fileBuffer = SDCardUtil::openFile(path, fileError);
    if (fileError != SDCardUtil::SDFileError::ERR_NONE || !fileBuffer)
    {
        // fail on data file
        // todo better error reporting
        isValid = false;
        SDCardUtil::closeFile(fileBuffer);
        return;
    }

    // Animations also require loop file validation
    if (!setup)
    {
        path[0] = '\0';
        SDCardUtil::getAnimationFilePath(index, path, true, false);

        File fileBuffer2 = SDCardUtil::openFile(path, fileError);
        if (fileError != SDCardUtil::SDFileError::ERR_NONE || !fileBuffer2)
        {
            // fail on loop data file
            // todo better error reporting
            isValid = false;
            SDCardUtil::closeFile(fileBuffer2);
            return;
        }
    }

    isValid = true;
}

bool SDCardCommandStreamDataSource::fillBufferChunk()
{
    /*
     * Flow: Open file -> Read chunks -> Handle file completion -> Repeat
     *
     * File opening: Opens setup file, or animation file, or loop file based on current state
     * Reading: Fill buffer with chunks while file has data and buffer has space
     * File completion handling:
     *   - Animation with looping: Switch from main file to loop file, then stop after loop
     *   - Animation without looping: Stop after main file
     *   - Setup: Stop after setup file
     * Returns: true if made progress, false if buffer full or no more data
     */
    // Core buffer filling logic shared by ESP32 task and non-ESP32 loop

    // open file if needed
    if (!currentFile)
    {
        char finalPath[MAX_FILE_PATH_SIZE];
        finalPath[0] = '\0';

        if (setup)
        {
            SDCardUtil::getSetupFilePath(finalPath);
        }
        else
        {
            SDCardUtil::getAnimationFilePath(index,
                                             finalPath,
                                             onLoop,
                                             false);
        }

        SDCardUtil::SDFileError fileError;
        currentFile = SDCardUtil::openFile(finalPath, fileError);

        if (fileError != SDCardUtil::SDFileError::ERR_NONE || !currentFile)
        {
            cardReadComplete = true;
            SDCardUtil::closeFile(currentFile);
            return false;
        }
    }

    if (SDCardUtil::safeAvailable(currentFile) && cardReadBuffer.getSpaceAvailable() >= TXT_BUFFER_READ_CHUNK_SIZE)
    {
        char tempBuffer[TXT_BUFFER_READ_CHUNK_SIZE];
        SDCardUtil::lockCard();
        int bytesRead = currentFile.readBytes(tempBuffer, TXT_BUFFER_READ_CHUNK_SIZE);
        SDCardUtil::unlockCard();

        for (int i = 0; i < bytesRead; i++)
        {
            cardReadBuffer.addChar(tempBuffer[i]);
        }

        if (!SDCardUtil::safeAvailable(currentFile))
        {
            // End of file: handle looping or completion
            if (shouldLoop)
            {
                if (!onLoop)
                {
                    // Transition from main animation to loop file
                    onLoop = true;
                    SDCardUtil::closeFile(currentFile);

                    char loopPath[MAX_FILE_PATH_SIZE];
                    loopPath[0] = '\0';
                    SDCardUtil::getAnimationFilePath(index, loopPath, true, false);
                    SDCardUtil::SDFileError fileError2;

                    currentFile = SDCardUtil::openFile(loopPath, fileError2);
                    if (fileError2 != SDCardUtil::SDFileError::ERR_NONE || !currentFile)
                    {
                        cardReadComplete = true;
                        SDCardUtil::closeFile(currentFile);
                        return false;
                    }
                }
                else
                {
                    cardReadComplete = true;
                    SDCardUtil::closeFile(currentFile);
                    return false;
                }
            }
            else
            {
                cardReadComplete = true;
                SDCardUtil::closeFile(currentFile);
                return false;
            }
        }
        return true;
    }

    return false;
}

#ifdef ESP32
void SDCardCommandStreamDataSource::startFillTask()
{
    xTaskCreate(
        fillTask,
        "SDCardFill",
        4096,
        this,
        1,
        (TaskHandle_t *)&fillTaskHandle);
}

// static so signature matches FreeRTOS xTaskCreate
void SDCardCommandStreamDataSource::fillTask(void *param)
{
    SDCardCommandStreamDataSource *self = static_cast<SDCardCommandStreamDataSource *>(param);

    while (!self->cardReadComplete)
    {
        // Keep reading blocks until the buffer is nearly full
        while (self->fillBufferChunk())
        {
            // fillBufferChunk returns false when buffer is full or no more data
        }

        // Block until buffer has space (producer/consumer sync)
        if (!self->cardReadComplete &&
            self->cardReadBuffer.getSpaceAvailable() < TXT_BUFFER_READ_CHUNK_SIZE)
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }


    if (self->currentFile)
    {
        Outgoing::printLine();
        SDCardUtil::closeFile(self->currentFile);
    }
    self->fillTaskHandle = nullptr;
    vTaskDelete(NULL);
}
#endif

void SDCardCommandStreamDataSource::updateOnLoop()
{
#ifndef ESP32
    // Non-ESP32: Fill buffer during main loop idle time
    if (!cardReadComplete && cardReadBuffer.getSpaceAvailable() >= TXT_BUFFER_READ_CHUNK_SIZE)
    {
        fillBufferChunk();
    }
#endif
    // ESP32: No-op, background task handles filling
}

void SDCardCommandStreamDataSource::getNextCommand(char *output, bool shouldLoop, bool peek)
{
    cardReadBuffer.getNextTxt(output, peek);

    if (!peek)
    {
        if (output[0] == '\0' && cardReadComplete)
        {
            dataComplete = true;
        }
#ifdef ESP32
        if (fillTaskHandle)
        {
            xTaskNotifyGive(fillTaskHandle);
        }
#endif
    }
}

void SDCardCommandStreamDataSource::reset()
{
#ifdef ESP32
    // Gracefully stop background task before reset
    if (fillTaskHandle)
    {
        cardReadComplete = true;
        xTaskNotifyGive((TaskHandle_t)fillTaskHandle);
        while (fillTaskHandle != nullptr)
        {
            vTaskDelay(1);
        }
    }
#endif

    onLoop = false;
    dataComplete = false;
    cardReadComplete = false;
    cardReadBuffer.clear();

#ifdef ESP32
    startFillTask(); // Restart background filling
#endif
}

#endif