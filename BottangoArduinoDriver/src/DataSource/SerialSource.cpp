// 
// 
// 

#include "SerialSource.h"
#include "../Outgoing.h"
#include "../BasicCommands.h"

void SerialSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}
	readData();
}

void SerialSource::init()
{
	_serialCommandBuffer[_serialCommandIdx] = '\0';

	Serial.begin(BAUD_RATE);

	Outgoing::printLine();
	Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
	Outgoing::printLine();
}

void SerialSource::readData()
{
	while (Serial.available() > 0)
	{
		_commandInProgress = true;

		char incomingChar = Serial.read();
		_timeOfLastChar = millis();

		// Look for end of string
		if (incomingChar == '\n')
		{
			_commandInProgress = false;
			_timeOfLastChar = 0;

			if (checkHash(_serialCommandBuffer))
			{
				// ToDo: There is a bug in Bottagno Desktop app during the handshake.
				// If the READY response is sent before the Handshake Response, the Handshake is processed, but it hangs in a weird state between "Handshake OK" and "Not OK"
				// Command is moved for the time being at the end of Parser.cpp onPhase() function.
				//Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
				_validDataAvailable = true;

				//Serial.printf("Got: %s\n", serialCommandBuffer);
			}
			else
			{
				Outgoing::printOutputStringPROGMEM(BasicCommands::HASH_FAIL);
			}
		}
		// Ignore \0
		else if (incomingChar == '\0')
		{
			// Do Nothing
		}		
		// Add char to receive buffer
		else if (_serialCommandIdx <= MAX_COMMAND_LENGTH - 2)
		{
			_serialCommandBuffer[_serialCommandIdx++] = incomingChar;
			_serialCommandBuffer[_serialCommandIdx] = '\0';
		}
		// Command to long
		else
		{
			Error::reportError_CmdTooLong(_serialCommandIdx);
			_serialCommandIdx = 0;
			_serialCommandBuffer[_serialCommandIdx] = '\0';

			_commandInProgress = false;
			_timeOfLastChar = 0;

			Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
		}
	}

	// Check for command timeout
	if (_commandInProgress && (millis() - _timeOfLastChar > READ_TIMEOUT))
	{
		Outgoing::printOutputStringPROGMEM(BasicCommands::TIMEOUT);

		_serialCommandIdx = 0;
		_serialCommandBuffer[_serialCommandIdx] = '\0';

		_commandInProgress = false;
		_timeOfLastChar = 0;
	}
}

bool SerialSource::tryConsumeData(char** out)
{
	if (_validDataAvailable)
	{
		*out = _serialCommandBuffer;
		_validDataAvailable = false;
		_serialCommandIdx = 0;
		return true;
	}

	return false;
}

void SerialSource::resetBuffer()
{
	_serialCommandIdx = 0;
	_serialCommandBuffer[_serialCommandIdx] = '\0';
	_validDataAvailable = false;
	_commandInProgress = false;
	//bufferLocked = false;
	_timeOfLastChar = 0;
}

bool SerialSource::checkHash(const char* cmdString)
{
	if (cmdString[0] == '\0')
	{
		return 0;
	}

	char c = cmdString[0];
	int idx = 0;

	// Scan forward to end of string
	while (c != '\0')
	{
		c = cmdString[idx++];
	}
	// Scan backward to find start of hash (don't hash the hash)
	idx -= 1; // One for 'h', one for ','

	while (idx > 0)
	{
		if (cmdString[idx] == ',' && cmdString[idx + 1] == 'h')
		{
			break;
		}
		idx--;
	}

	int hashStartIdx = idx + 2;

	idx -= 1; // For ','

	int hsh = 0;
	while (idx >= 0)
	{
		c = cmdString[idx--];
		hsh += c;
	}

	int ati = atoi(cmdString + hashStartIdx);

	return ati == hsh;
}