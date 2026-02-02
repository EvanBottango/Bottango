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

void AsciiCmdDecoder::init()
{
	splitData.splitCommandBuffer = splitCommandBuffer;
}

void AsciiCmdDecoder::decode()
{
	char* stringToSplit = nullptr;

	if (source->tryConsumeData(&stringToSplit) == false)
	{
		return;
	}

	validCommandAvailable = false;
	splitData.stringToSplit = stringToSplit;
	if (splitCommand(&splitData))
	{
		validCommandAvailable = true;
	}
}

bool AsciiCmdDecoder::splitCommand(/*char* stringToSplit*/splitCommandData* data) const
{
#ifdef ALLOW_SYNC_COMMANDS	
	// Check for sync command. If found, set validCommandAvailable to true and return.
	// The command is split when calling tryConsumeCommand() next time.
	if (strncmp_P(data->stringToSplit, BasicCommands::SYNC_COMMAND, 3) == 0)
	{
		data->syncCommandInProgress = true;

		// Start sync command and get first frame
		beginSyncCommand(data);
		return;
	}
#endif

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
	if (splitData.syncCommandInProgress)
	{
		if (hasMoreFrames(&splitData))
		{
			// Get next frame and split it
			getNextFrame(&splitData);
			splitCommand(&splitData);
			return splitCommandBuffer;
		}
		else
		{
			// Reset sync command state
			validCommandAvailable = false;
			splitData.syncCommandInProgress = false;
			return nullptr;
		}
		
	}
#endif

	if (validCommandAvailable)
	{
		validCommandAvailable = false;
		return splitData.splitCommandBuffer;
	}

	return nullptr;
}

unsigned long AsciiCmdDecoder::getStartTime(char* command)
{
	splitCommandData data;
	data.splitCommandBuffer = splitCommandBuffer;

	if (splitCommand(&data))
	{
		// First parameter is command name, second is start time
		if (data.splitCommandBuffer[2] != nullptr)
		{
			return Time::getLastSyncedTimeInMs() + strtoul(data.splitCommandBuffer[2], nullptr, 10);
		}
	}

	return 0;
}

unsigned long AsciiCmdDecoder::getEndTime(char* command)
{
	splitCommandData data;
	data.splitCommandBuffer = splitCommandBuffer;

	if (splitCommand(&data))
	{
		unsigned long endTime = 0;

		// First parameter is command name, third is end time
		if (data.splitCommandBuffer[3] != nullptr)
		{
			endTime = strtoul(data.splitCommandBuffer[3], nullptr, 10);

#ifdef ALLOW_SYNC_COMMANDS
			// With multi-frame sync commands, we need to check all frames for the latest end time
			while (hasMoreFrames(&data))
			{
				getNextFrame(&data);
				unsigned long subCmdTime = strtoul(data.splitCommandBuffer[3], nullptr, 10);
				if (subCmdTime > endTime)
				{
					endTime = subCmdTime;
				}
			}
#endif

			return Time::getLastSyncedTimeInMs() + endTime;
		}
	}

	return 0;
}

#ifdef ALLOW_SYNC_COMMANDS
void AsciiCmdDecoder::beginSyncCommand(/*char* stringToSplit*/splitCommandData* data) const
{
	data->expectNewCommand = true;

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

void AsciiCmdDecoder::getNextFrame(splitCommandData* data) const
{
	if (!data->nextFrameStart || *data->nextFrameStart == '\0')
	{
		data->currentCommand = nullptr;
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
		data->currentCommand = data->nextFrameStart;
		data->commandEnd = strchr(data->currentCommand, ',');
		if (!data->commandEnd)
		{
			data->currentCommand = nullptr;
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
	size_t commandLen = data->commandEnd - data->currentCommand;
	size_t frameDataLen = frameEnd - data->currentFrameStart;

	// Move frame data to the right to make space for command
	memmove(data->currentCommand + commandLen + 1, data->currentFrameStart, frameDataLen + 1); // +1 for \0

	// Set comma after command
	data->currentCommand[commandLen] = ',';

	// The complete frame starts at currentCommand
	//return data->currentCommand;
}

bool AsciiCmdDecoder::hasMoreFrames(splitCommandData* data) const
{
	return data->nextFrameStart && *data->nextFrameStart != '\0';
}
#endif