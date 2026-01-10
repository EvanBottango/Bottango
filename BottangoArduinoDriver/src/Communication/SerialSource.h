// SerialSource.h

#ifndef _SerialSource_h
#define _SerialSource_h

#include <Arduino.h>
#include "DataSource.h"

class SerialSource : public DataSource
{
	void readData() override;

	bool hasData() override;

	bool tryConsumeData(char* out) override;
private:
	bool checkHash(const char* cmdString);

	unsigned long timeOfLastChar = 0;
	bool commandInProgress = false;
	bool validCommandAvailable = false;
	//char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];

	char serialCommandBuffer[MAX_COMMAND_LENGTH];
	int serialCommandIdx = 0;
};


#endif

