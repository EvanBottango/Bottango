// 
// 
// 

#include "AsciiCmdDecoder.h"
#include "../Errors.h"
#include "../Outgoing.h"
#include "../../BottangoArduinoModules.h"
#include "../BasicCommands.h"

void AsciiCmdDecoder::onPhase(Phase p)
{
	// Only decode commands during the Logic phase
	if (p != Phase::Logic)
	{
		return;
	}

	decode();
}

void AsciiCmdDecoder::decode()
{
	char* stringToSplit = nullptr;

	if (source->tryConsumeData(&stringToSplit) == false)
	{
		return;
	}

#ifdef ALLOW_SYNC_COMMANDS	
	// Check for sync command. If found, set validCommandAvailable to true and return.
	// The command is split when calling tryConsumeCommand() next time.
	if (strncmp_P(stringToSplit, BasicCommands::SYNC_COMMAND, 3) == 0)
	{
		syncCommandInProgress = true;

		// Start sync command and get first frame
		beginSyncCommand(stringToSplit);
		return;
	}
#endif
	
	splitCommand(stringToSplit);
}

void AsciiCmdDecoder::splitCommand(char* stringToSplit)
{
	int idxResult = 0;
	char delimiters[] = ",";
	char* token = strtok(stringToSplit, delimiters);
	validCommandAvailable = false;

	while (token != NULL)
	{
		if (idxResult >= COMMANDS_PARAMS_SIZE) // Check buffer capacity
		{
			Error::reportError_TooManyParams(idxResult);
			return;
		}
		splitCommandBuffer[idxResult++] = token;
		token = strtok(NULL, delimiters);
	}

	validCommandAvailable = true;
}

char** AsciiCmdDecoder::tryConsumeCommand()
{
#ifdef ALLOW_SYNC_COMMANDS
	if (syncCommandInProgress)
	{
		if (hasMoreFrames())
		{
			// Get next frame and split it
			splitCommand(getNextFrame());
			return splitCommandBuffer;
		}
		else
		{
			// Reset sync command state
			validCommandAvailable = false;
			syncCommandInProgress = false;
			return nullptr;
		}
		
	}
#endif

	if (validCommandAvailable)
	{
		validCommandAvailable = false;
		return splitCommandBuffer;
	}

	return nullptr;
}

#ifdef ALLOW_SYNC_COMMANDS
void AsciiCmdDecoder::beginSyncCommand(char* stringToSplit)
{
	expectNewCommand = true;
	buffer = stringToSplit;

	// Remove hash at the end (everything after the last comma before 'h')
	char* lastComma = strrchr(buffer, ',');
	if (lastComma && lastComma[1] == 'h')
	{
		*lastComma = '\0';
	}

	// Skip MessageCommand (sSY,)
	char* pos = strchr(buffer, ',');
	if (!pos)
	{
		nextFrameStart = nullptr;
		return;
	}

	nextFrameStart = pos + 1; // Points to first command
}

char* AsciiCmdDecoder::getNextFrame()
{
	if (!nextFrameStart || *nextFrameStart == '\0')
	{
		return nullptr;
	}

	// Check if a new command starts (marked with asterisk)
	if (*nextFrameStart == '*')
	{
		nextFrameStart++; // Skip the asterisk
		expectNewCommand = true;
	}

	if (expectNewCommand)
	{
		// New command - find end of command
		currentCommand = nextFrameStart;
		commandEnd = strchr(currentCommand, ',');
		if (!commandEnd)
		{
			return nullptr;
		}

		// Place currentFrameStart after the command
		currentFrameStart = commandEnd + 1;
		expectNewCommand = false;
	}
	else
	{
		// No new command, use the previous one
		currentFrameStart = nextFrameStart;
	}

	// Find the end of the current frame (up to ; or end)
	char* frameEnd = currentFrameStart;
	while (*frameEnd != '\0' && *frameEnd != ';')
	{
		frameEnd++;
	}

	// Save what comes after this frame
	if (*frameEnd == ';')
	{
		nextFrameStart = frameEnd + 1;
		*frameEnd = '\0'; // Terminate the current frame
	}
	else
	{
		// End reached
		nextFrameStart = frameEnd;
	}

	// Now we build the frame: Command + , + FrameData
	// Calculate lengths
	size_t commandLen = commandEnd - currentCommand;
	size_t frameDataLen = frameEnd - currentFrameStart;

	// Move frame data to the right to make space for command
	memmove(currentCommand + commandLen + 1, currentFrameStart, frameDataLen + 1); // +1 for \0

	// Set comma after command
	currentCommand[commandLen] = ',';

	// The complete frame starts at currentCommand
	return currentCommand;
}

bool AsciiCmdDecoder::hasMoreFrames()
{
	return nextFrameStart && *nextFrameStart != '\0';
}
#endif