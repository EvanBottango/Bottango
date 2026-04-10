#include "CodeCommandStreamDataSource.h"
#include "../BottangoArduinoConfig.h"
#include "BottangoCore.h"

CodeCommandStreamDataSource::CodeCommandStreamDataSource(const char* const* dataArray, int8_t arrayLength) : AbstractCommandStreamDataSource()
{
	this->dataArray = dataArray;
	this->arrayLength = arrayLength;
	this->loopCharStream = nullptr;

	const char* ptr = (const char*)pgm_read_ptr(&this->dataArray[0]);
	this->currentDataStringLength = strlen_P(ptr);
}

CodeCommandStreamDataSource::CodeCommandStreamDataSource(const char* const* dataArray, int8_t arrayLength, const char* loopCharStream) : AbstractCommandStreamDataSource()
{
	this->dataArray = dataArray;
	this->arrayLength = arrayLength;
	this->loopCharStream = loopCharStream;

	const char* ptr = (const char*)pgm_read_ptr(&this->dataArray[0]);
	this->currentDataStringLength = strlen_P(ptr);
}

void CodeCommandStreamDataSource::getNextCommand(char* output, bool shouldLoop, bool peek)
{
	internalGetCharCommand(output, shouldLoop, !peek); // get the next command, write it to the output string
}

void CodeCommandStreamDataSource::internalGetCharCommand(char* output, bool shouldLoop, bool persistTravel)
{
	int8_t dataArrayIndexCache = dataArrayIndex;
	unsigned int travelCache = travel;
	bool dataCompleteCache = dataComplete;

	int outputIterator = 0;
	const char* currentString;

	if (dataArrayIndex == -1)
	{
		currentString = loopCharStream;
	}
	else
	{
		currentString = (const char*)pgm_read_ptr(&dataArray[dataArrayIndex]);
	}

	while (travel < currentDataStringLength && !dataComplete)
	{
		// get next char
		char nextChar = (char)pgm_read_byte(currentString + travel);
		travel++;

		// if got a full command
		if (nextChar == '\n' || outputIterator >= MAX_COMMAND_LENGTH)
		{
			// add string termination instead of newline
			output[outputIterator] = '\0';

			// at end of current array on last char of command?
			if (travel >= currentDataStringLength)
			{
				incrementArrayIndex(shouldLoop);
			}

			// go back to where we were before we found the command if not persist
			if (!persistTravel)
			{
				travel = travelCache;
				dataArrayIndex = dataArrayIndexCache;
				dataComplete = dataCompleteCache;
				if (dataArrayIndex == -1)
				{
					currentString = loopCharStream;
					currentDataStringLength = strlen_P(this->loopCharStream);
				}
				else
				{
					currentString = (const char*)pgm_read_ptr(&dataArray[dataArrayIndex]);
					currentDataStringLength = strlen_P(currentString);
				}
			}

			// command found, we're done
			return;
		}

		// add char to ouput
		output[outputIterator] = nextChar;
		outputIterator++;

		// at end of current array mid command?
		if (travel >= currentDataStringLength)
		{
			incrementArrayIndex(shouldLoop);
			// make sure to grab the new string
			if (dataArrayIndex == -1)
			{
				currentString = loopCharStream;
			}
			else
			{
				currentString = (const char*)pgm_read_ptr(&dataArray[dataArrayIndex]);
			}
		}
	}
}

void CodeCommandStreamDataSource::incrementArrayIndex(bool shouldLoop)
{
	travel = 0;

	// if at end of loop, set complete and we're done
	if (shouldLoop && dataArrayIndex == -1)
	{
		dataComplete = true;
		return;
	}
	// in regular animation
	else
	{
		dataArrayIndex++;
		// if at end of animation
		if (dataArrayIndex >= arrayLength)
		{
			// go to loop
			if (shouldLoop)
			{
				unsigned int loopLength = strlen_P(this->loopCharStream);
				if (loopLength > 0)
				{
					dataArrayIndex = -1;
					currentDataStringLength = loopLength;
				}
				else
				{
					dataComplete = true;
				}
				return;
			}
			// or all done if no loop and exit
			else
			{
				dataComplete = true;
				return;
			}
		}
	}

	// reset string length
	const char* ptr = (const char*)pgm_read_ptr(&this->dataArray[dataArrayIndex]);
	this->currentDataStringLength = strlen_P(ptr);
}

void CodeCommandStreamDataSource::reset()
{
	travel = 0;
	dataArrayIndex = 0;
	dataComplete = false;
	const char* ptr = (const char*)pgm_read_ptr(&this->dataArray[dataArrayIndex]);
	this->currentDataStringLength = strlen_P(ptr);
}
