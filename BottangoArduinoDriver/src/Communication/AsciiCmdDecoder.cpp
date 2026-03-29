// 
// 
// 

#include "AsciiCmdDecoder.h"
#include "../Errors.h"
#include "../Modules/Outgoing.h"
#include "../../BottangoArduinoModules.h"
#include "../BasicCommands.h"
#include "../System/SystemStatus.h"

void AsciiCmdDecoder::onPhase(Phase p)
{
	// Only decode commands during the Logic phase
	if (p != Phase::Logic)
	{
		return;
	}

	decode();
}

void AsciiCmdDecoder::init()
{
	
}

void AsciiCmdDecoder::decode()
{
	char* stringToSplit = nullptr;

	// Check both sources, but the primary source always has priority
	if (_source->tryConsumeData(&stringToSplit))
	{
		_sourceIsUsbSerial = true;
	}
	else if (_secondarySource && _secondarySource->tryConsumeData(&stringToSplit))
	{
		_sourceIsUsbSerial = false;
	}
	else
	{
		return;
	}

	_validCommandAvailable = false;
	_splitData.stringToSplit = stringToSplit;
	if (splitCommand(&_splitData))
	{
		_validCommandAvailable = true;
	}
}

bool AsciiCmdDecoder::splitCommand(SplitCommandData* data) const
{
#ifdef RELAY_SUPPORTED
	if (strncmp_P(data->stringToSplit, BasicCommands::PASS_TO_RELAY, 2) == 0)
	{
		return splitRelayCommand(data);
	}
#endif

#ifdef ALLOW_SYNC_COMMANDS
	// Check for sync command. If found, set validCommandAvailable to true and return.
	// The command is split when calling tryConsumeCommand() next time.
	if (strncmp_P(data->stringToSplit, BasicCommands::SYNC_COMMAND, 3) == 0)
	{
		data->syncCommandInProgress = true;

		// Start sync command and get first frame
		beginSyncCommand(data);
		return true;
	}
#endif // ALLOW_SYNC_COMMANDS

	int idxResult = 0;
	char delimiters[] = ",";
	char* token = strtok(data->stringToSplit, delimiters);
	//validCommandAvailable = false;

	while (token != NULL)
	{
		if (idxResult >= COMMANDS_PARAMS_SIZE) // Check buffer capacity
		{
			Error::reportError_TooManyParams(idxResult);
			return false;
		}
		data->splitCommandBuffer[idxResult++] = token;
		token = strtok(NULL, delimiters);
	}

	return true;
}

char** AsciiCmdDecoder::tryConsumeCommand()
{
#ifdef ALLOW_SYNC_COMMANDS
	if (_splitData.syncCommandInProgress)
	{
		if (hasMoreFrames(&_splitData))
		{
			// Get next frame and split it
			getNextFrame(&_splitData);
			splitCommand(&_splitData);
			return _splitData.splitCommandBuffer;
		}
		else
		{
			// Reset sync command state
			_validCommandAvailable = false;
			return nullptr;
		}
		
	}
#endif // ALLOW_SYNC_COMMANDS

	if (_validCommandAvailable)
	{
		_validCommandAvailable = false;
		return _splitData.splitCommandBuffer;
	}

	return nullptr;
}

#ifdef ALLOW_SYNC_COMMANDS
void AsciiCmdDecoder::beginSyncCommand(SplitCommandData* data) const
{
	data->expectNewCommand = true;

	// Flip pointers
	data->syncCommandToSplit = data->stringToSplit;

	// Remove hash at the end (everything after the last comma before 'h')
	char* lastComma = strrchr(data->stringToSplit, ',');
	if (lastComma && lastComma[1] == 'h')
	{
		*lastComma = '\0';
	}

	// Skip MessageCommand (sSY,)
	char* pos = strchr(data->stringToSplit, ',');
	if (!pos)
	{
		data->nextFrameStart = nullptr;
		return;
	}

	data->nextFrameStart = pos + 1; // Points to first command
}

void AsciiCmdDecoder::getNextFrame(SplitCommandData* data) const
{
	if (!data->nextFrameStart || *data->nextFrameStart == '\0')
	{
		data->stringToSplit = nullptr;
	}

	// Check if a new command starts (marked with asterisk)
	if (*data->nextFrameStart == '*')
	{
		data->nextFrameStart++; // Skip the asterisk
		data->expectNewCommand = true;
	}

	if (data->expectNewCommand)
	{
		// New command - find end of command
		data->stringToSplit = data->nextFrameStart;
		data->commandEnd = strchr(data->stringToSplit, ',');
		if (!data->commandEnd)
		{
			data->stringToSplit = nullptr;
		}

		// Place currentFrameStart after the command
		data->currentFrameStart = data->commandEnd + 1;
		data->expectNewCommand = false;
	}
	else
	{
		// No new command, use the previous one
		data->currentFrameStart = data->nextFrameStart;
	}

	// Find the end of the current frame (up to ; or end)
	char* frameEnd = data->currentFrameStart;
	while (*frameEnd != '\0' && *frameEnd != ';')
	{
		frameEnd++;
	}

	// Save what comes after this frame
	if (*frameEnd == ';')
	{
		data->nextFrameStart = frameEnd + 1;
		*frameEnd = '\0'; // Terminate the current frame
	}
	else
	{
		// End reached
		data->nextFrameStart = frameEnd;
	}

	// Now we build the frame: Command + , + FrameData
	// Calculate lengths
	size_t commandLen = data->commandEnd - data->stringToSplit;
	size_t frameDataLen = frameEnd - data->currentFrameStart;

	// Move frame data to the right to make space for command
	memmove(data->stringToSplit + commandLen + 1, data->currentFrameStart, frameDataLen + 1); // +1 for \0

	// Set comma after command
	data->stringToSplit[commandLen] = ',';
}

bool AsciiCmdDecoder::hasMoreFrames(SplitCommandData* data) const
{
	bool returnValue = data->nextFrameStart && *data->nextFrameStart != '\0';

	if (!returnValue)
	{
		// No more frames, reset sync command state
		data->syncCommandInProgress = false;
	}

	return returnValue;
}
#endif // ALLOW_SYNC_COMMANDS

#ifdef RELAY_SUPPORTED
bool AsciiCmdDecoder::splitRelayCommand(SplitCommandData* data) const
{
	// Find first comma (after sR)
	char* firstComma = strchr(data->stringToSplit, ',');
	if (!firstComma)
	{
		return false;
	}

	// Find second comma (after ID)
	char* secondComma = strchr(firstComma + 1, ',');
	if (!secondComma)
	{
		return false;
	}

	// Find last comma (for hash split)
	char* lastComma = strrchr(secondComma + 1, ',');
	if (!lastComma || lastComma <= secondComma)
	{
		return false;
	}

	// Set splitCommandBuffer[0] = sR
	size_t len0 = firstComma - data->stringToSplit;
	static char sRbuffer[8]; // enough for sR\0
	strncpy(sRbuffer, data->stringToSplit, len0);
	sRbuffer[len0] = '\0';
	data->splitCommandBuffer[0] = sRbuffer;

	// Set splitCommandBuffer[1] = ID
	size_t len1 = secondComma - (firstComma + 1);
	static char idBuffer[8]; // enough for id + \0
	strncpy(idBuffer, firstComma + 1, len1);
	idBuffer[len1] = '\0';
	data->splitCommandBuffer[1] = idBuffer;

	if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
	{
		// Set splitCommandBuffer[2] = everything after 2nd comma
		// except trim last char if it's a \n newline
		static char offlinePayloadBuffer[MAX_COMMAND_LENGTH];
		strcpy(offlinePayloadBuffer, secondComma + 1);

		size_t payloadLen = strlen(offlinePayloadBuffer);
		if (payloadLen > 0 && offlinePayloadBuffer[payloadLen - 1] == '\n')
		{
			offlinePayloadBuffer[payloadLen - 1] = '\0';
		}

		data->splitCommandBuffer[2] = offlinePayloadBuffer;

		// Set splitCommandBuffer[3] = to an empty string, since there's no hash in an offline command
		static char emptyString[] = "";
		data->splitCommandBuffer[3] = emptyString;
	}
	else
	{
		// Set splitCommandBuffer[2] = everything between 2nd and last comma
		size_t len2 = lastComma - (secondComma + 1);
		static char payloadBuffer[MAX_COMMAND_LENGTH];
		strncpy(payloadBuffer, secondComma + 1, len2);
		payloadBuffer[len2] = '\0';
		data->splitCommandBuffer[2] = payloadBuffer;

		// Set splitCommandBuffer[3] = hash (after last comma)
		data->splitCommandBuffer[3] = lastComma + 1;
	}

	return true;
}
#endif