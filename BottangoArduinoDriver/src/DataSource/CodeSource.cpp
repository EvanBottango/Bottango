#include "CodeSource.h"

#ifdef USE_CODE_COMMAND_STREAM
#include "../GeneratedCodeAnimations.h"
#include <Arduino.h>

void CodeSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}
}

void CodeSource::init()
{
	_commandBuffer[0] = '\0';
}

bool CodeSource::openSetup()
{
	resetBuffer();
	_dataArray = GeneratedCodeAnimations::getSetup();
	_arrayLength = 1;

	const char* ptr = (const char*)pgm_read_ptr(&_dataArray[0]);
	_currentDataStringLength = strlen_P(ptr);

	return true;
}

bool CodeSource::openAnimation(uint8_t animIndex, bool loop)
{
	if (animIndex >= GeneratedCodeAnimations::getAnimationCount())
	{
		return false;
	}

	resetBuffer();
	_dataArray = GeneratedCodeAnimations::getAnimationDataByIndex(animIndex);
	_shouldLoop = loop;

	const char* ptr = (const char*)pgm_read_ptr(&_dataArray[0]);
	_currentDataStringLength = strlen_P(ptr);

	return true;
}

void CodeSource::prepareNextCommand()
{
	_validDataAvailable = getNextCommand(_commandBuffer);
}

bool CodeSource::peekNextCommand(char* out)
{
	return getNextCommand(out, true);
}

bool CodeSource::tryConsumeData(char** out)
{
	if (_validDataAvailable)
	{
		*out = _commandBuffer;
		_validDataAvailable = false;
		return true;
	}

	return false;
}

void CodeSource::resetBuffer()
{
	_loopCharStream = nullptr;
	_shouldLoop = false;
	_dataArrayIndex = 0;
	_travel = 0;
	_dataComplete = false;
	_shouldLoop = false;
	_currentDataStringLength = 0;
	_commandBuffer[0] = '\0';
}

bool CodeSource::getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const
{
	if (animIndex >= GeneratedCodeAnimations::getAnimationCount())
	{
		return false;
	}

	const uint16_t* parsedValue = GeneratedCodeAnimations::getConfigValues(animIndex);
	parseConfiguration(config, parsedValue);

	return true;
}

void CodeSource::parseConfiguration(AnimationConfiguration* config, const uint16_t* parsedValue) const
{
	if (parsedValue[0] > 0)
	{
		config->flags |= ANIM_PLAY_ON_START_FLAG;
	}

	if (parsedValue[1] > 0)
	{
		config->flags |= ANIM_LOOP_ON_START_FLAG;
	}

	if (parsedValue[2] > 0)
	{
		config->flags |= ANIM_IDLE_FLAG;
	}

	config->playOnPin = parsedValue[3];

	if (parsedValue[4] > 0)
	{
		config->flags |= ANIM_LOOPING_FLAG;
	}

	if (parsedValue[5] == 0)
	{
		config->flags |= ANIM_PLAY_ON_PIN_LOW_FLAG;
	}
	else if (parsedValue[5] == 1)
	{
		config->flags |= ANIM_PLAY_ON_PIN_HIGH_FLAG;
	}
	else if (parsedValue[5] == 2)
	{
		config->flags |= ANIM_PLAY_ON_PIN_ANALOG_FLAG;
	}

	config->buttonLadderMin = parsedValue[6];
	config->buttonLadderMax = parsedValue[7];
}

bool CodeSource::getNextCommand(char* buffer, bool peek)
{
	int8_t dataArrayIndexCache = _dataArrayIndex;
	unsigned int travelCache = _travel;
	bool dataCompleteCache = _dataComplete;

	int outputIterator = 0;
	const char* currentString;

	if (_dataArrayIndex == -1)
	{
		currentString = _loopCharStream;
	}
	else
	{
		currentString = (const char*)pgm_read_ptr(&_dataArray[_dataArrayIndex]);
	}

	while (_travel < _currentDataStringLength && !_dataComplete)
	{
		// get next char
		char nextChar = (char)pgm_read_byte(currentString + _travel);
		_travel++;

		// if got a full command
		// ToDo: Can we be 100% sure, that we never hit the max command length without a newline in the data?
		// This should throw a "Command to long" error, and not be treated as a valid command
		if (nextChar == '\n' || outputIterator >= MAX_COMMAND_LENGTH)
		{
			// add string termination instead of newline
			buffer[outputIterator] = '\0';

			// at end of current array on last char of command?
			if (_travel >= _currentDataStringLength)
			{
				incrementArrayIndex();
			}

			// go back to where we were before we found the command if peeking
			if (peek)
			{
				_travel = travelCache;
				_dataArrayIndex = dataArrayIndexCache;
				_dataComplete = dataCompleteCache;
				if (_dataArrayIndex == -1)
				{
					currentString = _loopCharStream;
					_currentDataStringLength = strlen_P(_loopCharStream);
				}
				else
				{
					currentString = (const char*)pgm_read_ptr(&_dataArray[_dataArrayIndex]);
					_currentDataStringLength = strlen_P(currentString);
				}
			}

			// if we got a full command, return it
			// This is to prevent the code from returning a empty "\0"
			if (outputIterator >= 1)
			{
				return true;
			}
			return false;
		}

		// add char to ouput
		buffer[outputIterator] = nextChar;
		outputIterator++;

		// at end of current array mid command?
		if (_travel >= _currentDataStringLength)
		{
			incrementArrayIndex();
			// make sure to grab the new string
			if (_dataArrayIndex == -1)
			{
				currentString = _loopCharStream;
			}
			else
			{
				currentString = (const char*)pgm_read_ptr(&_dataArray[_dataArrayIndex]);
			}
		}
	}

	return false;
}

void CodeSource::incrementArrayIndex()
{
	_travel = 0;

	// if at end of loop, set complete and we're done
	if (_shouldLoop && _dataArrayIndex == -1)
	{
		_dataComplete = true;
		return;
	}
	// in regular animation
	else
	{
		_dataArrayIndex++;
		// if at end of animation
		if (_dataArrayIndex >= _arrayLength)
		{
			// go to loop
			if (_shouldLoop)
			{
				unsigned int loopLength = strlen_P(_loopCharStream);
				if (loopLength > 0)
				{
					_dataArrayIndex = -1;
					_currentDataStringLength = loopLength;
				}
				else
				{
					_dataComplete = true;
				}
				return;
			}
			// or all done if no loop and exit
			else
			{
				_dataComplete = true;
				return;
			}
		}
	}

	// reset string length
	const char* ptr = (const char*)pgm_read_ptr(&_dataArray[_dataArrayIndex]);
	_currentDataStringLength = strlen_P(ptr);
}

#endif // USE_CODE_COMMAND_STREAM