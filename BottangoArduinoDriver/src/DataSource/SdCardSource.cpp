#include "../../BottangoArduinoModules.h"
#ifdef USE_SD_CARD_COMMAND_STREAM

#include "SdCardSource.h"
#include "SDCardUtil.h"
#include "../System/SystemStatus.h"
#include "../../BottangoArduinoCallbacks.h"
#include "../PersistentConfigUtil.h"

#include "Logger/Logger.h"

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

SdCardSource::~SdCardSource()
{
	// Graceful task shutdown with timeout to prevent hangs

#ifdef ESP32
	stopFillTask();
#endif
}

void SdCardSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}

#ifndef ESP32
	updateOnLoop();
#endif
}

void SdCardSource::init()
{
	_commandBuffer[0] = '\0';
}

bool SdCardSource::openSetup()
{
	// Open setup file, only if another file is not already open
	if (!_currentFile)
	{
		_dataComplete = false;
		_fileReadComplete = false;
		_shouldLoop = false;
		LOG_DEBUG("SdCard", "openSetup()", "Opening Setup File");
		LOG_DEBUG("SdCard", "openSetup()", "Marking _fileReadComplete = false");

		char finalPath[MAX_FILE_PATH_SIZE];
		finalPath[0] = '\0';

		// Get setup file path
		SDCardUtil::getSetupFilePath(finalPath);

		// Open setup file and check for errors
		SDCardUtil::SDFileError fileError;
		_currentFile = SDCardUtil::openFile(finalPath, fileError);
		if (fileError != SDCardUtil::SDFileError::ERR_NONE || !_currentFile)
		{
			SDCardUtil::closeFile(_currentFile);
			return false;
		}

#ifdef ESP32
		startFillTask(); // ESP32: Background task fills buffer
#else
		updateOnLoop(); // Non-ESP32: Buffer filled during main loop via updateOnLoop(), but needs to be primed
#endif
	}
	
	return false;
}

bool SdCardSource::openAnimation(uint8_t animIndex, bool loop)
{
	// Open animation file, only if another file is not already open
	if (!_currentFile)
	{
		_dataComplete = false;
		_fileReadComplete = false;
		LOG_DEBUG("SdCard", "openAnimation()", "Opening Animation: %u", animIndex);
		LOG_DEBUG("SdCard", "openAnimation()", "Marking _fileReadComplete = false");
		_onLoop = false;

		_index = animIndex;
		_shouldLoop = loop;

		char finalPath[MAX_FILE_PATH_SIZE];
		finalPath[0] = '\0';

		// Get animation file path
		SDCardUtil::getAnimationFilePath(_index, finalPath, loop, false);

		// Open animation file and check for errors
		SDCardUtil::SDFileError fileError;
		_currentFile = SDCardUtil::openFile(finalPath, fileError);
		if (fileError != SDCardUtil::SDFileError::ERR_NONE || !_currentFile)
		{
			SDCardUtil::closeFile(_currentFile);
			return false;
		}

#ifdef ESP32
		startFillTask(); // ESP32: Background task fills buffer
#else
		updateOnLoop(); // Non-ESP32: Buffer filled during main loop via updateOnLoop(), but needs to be primed
#endif
		return true;
	}

	return false;
}

void SdCardSource::prepareNextCommand()
{
	// Read the next line from the currently opened file until '\n'
	getNextCommand(_commandBuffer);

#ifndef ESP32
	// got something empty back? Let's update data source and try one more time
	if (_commandBuffer[0] == '\0')
	{
		updateOnLoop();
		getNextCommand(_commandBuffer);
	}
#endif
}

bool SdCardSource::peekNextCommand(char* out)
{
	return getNextCommand(out, true);
}

#ifdef ESP32
void SdCardSource::startFillTask()
{
	//_commandQueue = xQueueCreate(4, sizeof(SdCommand));

	xTaskCreate(
		fillTask,
		"SDCardFill",
		4096,
		this,
		1,
		(TaskHandle_t*)&_fillTaskHandle);
}

void SdCardSource::stopFillTask()
{
	_fileReadComplete = true;
	LOG_DEBUG("SdCard", "stopFillTask()", "Marking _fileReadComplete = true");
	if (_fillTaskHandle)
	{
		// Notify the task to wake up and see the cardReadComplete flag
		xTaskNotifyGive(_fillTaskHandle);

		// Wait for the task to clean itself up (with timeout)
		int timeout = 250;
		while (_fillTaskHandle != nullptr && timeout > 0)
		{
			vTaskDelay(1);
			timeout--;
		}

		// Force delete if task didn't exit gracefully
		if (_fillTaskHandle != nullptr)
		{
			vTaskDelete(_fillTaskHandle);
			_fillTaskHandle = nullptr;

			if (_currentFile)
			{
				SDCardUtil::closeFile(_currentFile);
			}
		}
	}
}

// static so signature matches FreeRTOS xTaskCreate
void SdCardSource::fillTask(void* param)
{
	SdCardSource* self = static_cast<SdCardSource*>(param);

	while (!self->_fileReadComplete)
	{
		// Keep reading blocks until the buffer is nearly full
		while (self->fillBufferChunk())
		{
			// fillBufferChunk returns false when buffer is full or no more data
		}

		// Block until buffer has space (producer/consumer sync)
		if (!self->_fileReadComplete &&
			self->_cardReadBuffer.getSpaceAvailable() < TXT_BUFFER_READ_CHUNK_SIZE)
		{
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
	}

	if (self->_currentFile)
	{
		//Outgoing::printLine();
		SDCardUtil::closeFile(self->_currentFile);
	}
	LOG_DEBUG("SdCard", "fillTask()", "File read complete, exiting task");
	self->_fillTaskHandle = nullptr;
	vTaskDelete(NULL);
}
#else
void SdCardSource::updateOnLoop()
{
	// Non-ESP32: Fill buffer during main loop idle time
	if (!_fileReadComplete && _cardReadBuffer.getSpaceAvailable() >= TXT_BUFFER_READ_CHUNK_SIZE)
	{
		fillBufferChunk();
	}

	// ESP32: No-op, background task handles filling
}
#endif

bool SdCardSource::fillBufferChunk()
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
	/*if (!currentFile)
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
	}*/

	if (SDCardUtil::safeAvailable(_currentFile) && _cardReadBuffer.getSpaceAvailable() >= TXT_BUFFER_READ_CHUNK_SIZE)
	{
		char tempBuffer[TXT_BUFFER_READ_CHUNK_SIZE];
		SDCardUtil::lockCard();
		int bytesRead = _currentFile.readBytes(tempBuffer, TXT_BUFFER_READ_CHUNK_SIZE);
		SDCardUtil::unlockCard();

		// Add read data to circular buffer
		for (int i = 0; i < bytesRead; i++)
		{
			_cardReadBuffer.addChar(tempBuffer[i]);
		}

		// No more data available?
		if (!SDCardUtil::safeAvailable(_currentFile))
		{
			// End of file: handle looping or completion
			if (_shouldLoop)
			{
				if (!_onLoop)
				{
					// Transition from main animation to loop file
					_onLoop = true;
					SDCardUtil::closeFile(_currentFile);

					char loopPath[MAX_FILE_PATH_SIZE];
					loopPath[0] = '\0';
					SDCardUtil::getAnimationFilePath(_index, loopPath, true, false);
					SDCardUtil::SDFileError fileError2;

					_currentFile = SDCardUtil::openFile(loopPath, fileError2);
					if (fileError2 != SDCardUtil::SDFileError::ERR_NONE || !_currentFile)
					{
						_fileReadComplete = true;
						SDCardUtil::closeFile(_currentFile);
						return false;
					}
				}
				else
				{
					_fileReadComplete = true;
					SDCardUtil::closeFile(_currentFile);
					return false;
				}
			}
			else
			{
				_fileReadComplete = true;
				LOG_DEBUG("SdCard", "fillBufferChunk()", "Marking _fileReadComplete = true");
				SDCardUtil::closeFile(_currentFile);
				return false;
			}
		}
		return true;
	}

	return false;
}

bool SdCardSource::tryConsumeData(char** out)
{
	if (_validDataAvailable)
	{
		*out = _commandBuffer;
		_validDataAvailable = false;
		return true;
	}

	return false;
}

bool SdCardSource::getNextCommand(char* buffer, bool peek)
{
	if (_dataComplete)
	{
		return false;
	}

	// Copy a whole command (sC,12,...\n\0) into the command buffer
	// ToDo: This funcion can also return \0, if the buffer is empty (Underflow-Error). The following code does not check for that case?
	_cardReadBuffer.getNextTxt(buffer, peek);

	/*if (peek && buffer[0] == '\0' && _fileReadComplete)
	{
		_dataComplete = true;
		return false;
	}
	else if (peek)
	{
		return true;
	}*/

	// If we got no data and fillBufferChunk() marked the files as completely read
	if (buffer[0] == '\0' || buffer[0] == '\r' || buffer[0] == '\n')
	{
		// Buffer Underflow-Protection. Only mark data as complete if we actually have read everything from the file
		if (_fileReadComplete)
		{
			_dataComplete = true;
			LOG_DEBUG("SdCard", "getNextCommand()", "Marking _dataComplete = true");			
		}

		return false;
	}
	else
	{
		// Only mark data as available, if not peeking
		// Also, only notify the read task, if we actually read anything from the buffer, to avoid unnecessary task wakeups
		if (!peek)
		{
			_validDataAvailable = true;

#ifdef ESP32
			if (_fillTaskHandle)
			{
				xTaskNotifyGive(_fillTaskHandle);
			}
#endif // ESP32
		}
	}

	return true;
}

void SdCardSource::resetBuffer()
{
#ifdef ESP32
	// Gracefully stop background task before reset
	stopFillTask();
#endif // ESP32

	_onLoop = false;
	_dataComplete = false;
	_fileReadComplete = false;
	LOG_DEBUG("SdCard", "resetBuffer()", "Marking _fileReadComplete = false");
	_cardReadBuffer.clear();
	_commandBuffer[0] = '\0';

/*#ifdef ESP32
	startFillTask(); // Restart background filling
#endif*/
}

bool SdCardSource::getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const
{
	char path[MAX_FILE_PATH_SIZE];
	SDCardUtil::SDFileError fileError;

	// first verify existance of all required files
	path[0] = '\0';
	SDCardUtil::getAnimationFilePath(animIndex, path, false, true); // Get and open CONFIG file

	bool exists = SDCardUtil::fileExists(path, fileError);
	if (!exists)
	{
		return false;
	}

	File configFile = SDCardUtil::openFile(path, fileError);

	// error case on config
	if (fileError != SDCardUtil::SDFileError::ERR_NONE)
	{
		// TODO
		// better error case handling here!
		SDCardUtil::closeFile(configFile);
		return false;
	}

	parseConfiguration(configFile, config);

	// done with the file
	SDCardUtil::closeFile(configFile);

	return true;
}

void SdCardSource::parseConfiguration(File configFile, AnimationConfiguration* config) const
{
	byte lineIndex = 0;

	while (SDCardUtil::safeAvailable(configFile))
	{
		SDCardUtil::lockCard();
		// start of the line
		char c = configFile.read();
		SDCardUtil::unlockCard();

		// skip this line, it's a comment
		if (c == '/')
		{
			while (SDCardUtil::safeAvailable(configFile))
			{
				SDCardUtil::lockCard();
				c = configFile.read();
				SDCardUtil::unlockCard();
				if (c == '\n' || c == '\r')
				{
					break;
				}
			}
		}

		// skip this line, it's blank
		if (c == '\n' || c == '\r')
		{
			continue;
		}

		// valid line
		char value[10];
		value[0] = c;
		for (int i = 1; i < 10; i++)
		{
			if (SDCardUtil::safeAvailable(configFile))
			{
				SDCardUtil::lockCard();
				char cNext = configFile.read();
				SDCardUtil::unlockCard();
				if (cNext == '\n' || cNext == '\r')
				{
					value[i] = '\0';
					break;
				}
				else
				{
					value[i] = cNext;
					if (i == 8)
					{
						value[9] = '\0';
					}
				}
			}
		}

		int parsedValue = atoi(value);

		switch (lineIndex)
		{
		case 0:
			if (parsedValue > 0)
			{
				config->flags |= ANIM_PLAY_ON_START_FLAG;
			}
			break;
		case 1:
			if (parsedValue > 0)
			{
				config->flags |= ANIM_LOOP_ON_START_FLAG;
			}
			break;
		case 2:
			if (parsedValue > 0)
			{
				config->flags |= ANIM_IDLE_FLAG;
			}
			break;
		case 3:
			config->playOnPin = parsedValue;
			break;
		case 4:
			if (parsedValue > 0)
			{
				config->flags |= ANIM_LOOPING_FLAG;
			}
			break;
		case 5:
			if (parsedValue == 0)
			{
				config->flags |= ANIM_PLAY_ON_PIN_LOW_FLAG;
			}
			else if (parsedValue == 1)
			{
				config->flags |= ANIM_PLAY_ON_PIN_HIGH_FLAG;
			}
			else if (parsedValue == 2)
			{
				config->flags |= ANIM_PLAY_ON_PIN_ANALOG_FLAG;
			}
			break;
		case 6:
			config->buttonLadderMin = parsedValue;
			break;
		case 7:
			config->buttonLadderMax = parsedValue;

			break;
		}

		lineIndex++;
	}
}

#endif // USE_SD_CARD_COMMAND_STREAM