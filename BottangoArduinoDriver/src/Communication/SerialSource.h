// SerialSource.h

#ifndef _SerialSource_h
#define _SerialSource_h

#include <Arduino.h>
#include "DataSource.h"

class SerialSource : public DataSource
{
public:
	void onPhase(Phase p) override;

	void init() override;

	void readData() override;

	bool tryConsumeData(char** out) override;

	//void releaseBuffer();

	void resetBuffer() override;

private:
	bool checkHash(const char* cmdString);

	unsigned long timeOfLastChar = 0;
	bool commandInProgress = false;

	char serialCommandBuffer[MAX_COMMAND_LENGTH];
	int serialCommandIdx = 0;
	//bool bufferLocked = false;
};


#endif