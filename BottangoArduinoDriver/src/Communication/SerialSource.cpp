// 
// 
// 

#include "SerialSource.h"
#include "../Outgoing.h"


void SerialSource::readData()
{
	while (Serial.available() > 0)
	{
		commandInProgress = true;

		char incomingChar = Serial.read();
		timeOfLastChar = millis();

		// Look for end of string
		if (incomingChar == '\n')
		{
			commandInProgress = false;

			if (checkHash(serialCommandBuffer))
			{
				Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
				validDataAvailable = true;
			}
			else
			{
				Outgoing::printOutputStringPROGMEM(BasicCommands::HASH_FAIL);
			}

			serialCommandIdx = 0;
			serialCommandBuffer[serialCommandIdx] = '\0';
		}
		// Ignore \0
		else if (incomingChar == '\0')
		{
			// Do Nothing
		}		
		// Add char to receive buffer
		else if (serialCommandIdx <= MAX_COMMAND_LENGTH - 2)
		{
			serialCommandBuffer[serialCommandIdx++] = incomingChar;
			serialCommandBuffer[serialCommandIdx] = '\0';
		}
		// Command to long
		else
		{
			Error::reportError_CmdTooLong(serialCommandIdx);
			serialCommandIdx = 0;
			serialCommandBuffer[serialCommandIdx] = '\0';

			commandInProgress = false;
			timeOfLastChar = 0;

			Outgoing::printOutputStringPROGMEM(BasicCommands::READY);
		}
	}

	// Check for command timeout
	if (commandInProgress && (millis() - timeOfLastChar > READ_TIMEOUT))
	{
		Outgoing::printOutputStringPROGMEM(BasicCommands::TIMEOUT);

		serialCommandIdx = 0;
		serialCommandBuffer[serialCommandIdx] = '\0';

		commandInProgress = false;
		timeOfLastChar = 0;
	}
}

bool SerialSource::tryConsumeData(char* out)
{
	if (validDataAvailable)
	{
		out = serialCommandBuffer;
		validDataAvailable = false;
		return true;
	}

	return false;
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