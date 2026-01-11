// 
// 
// 

#include "AsciiFrameDecoder.h"


void AsciiFrameDecoder::decode(DataSource* source)
{
	char* stringToSplit = nullptr;

	if (source->tryConsumeData(stringToSplit) == false)
	{
		return;
	}

	int idxResult = 0;
	char delimiters[] = ",";
	char* token = strtok(stringToSplit, delimiters);
	validFrameAvailable = false;

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

	validFrameAvailable = true;

	//paramsCount = idxResult;
};