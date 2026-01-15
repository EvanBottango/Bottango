// 
// 
// 

#include "AsciiCmdDecoder.h"
#include "../Errors.h"
#include "../Outgoing.h"

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

	//Outgoing::printOutputStringMem("Decoding Command\n");
	//Serial.printf("Decoding Command: %s\n", stringToSplit);
	//Outgoing::printLine();

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

	/*Serial.printf("Decode 0: %s\n", splitCommandBuffer[0]);
	Serial.printf("Decode 1: %s\n", splitCommandBuffer[1]);
	Serial.printf("Decode 2: %s\n", splitCommandBuffer[2]);

	Outgoing::printOutputStringMem("Decoding Done\n");
	Serial.flush();*/

	//paramsCount = idxResult;
};