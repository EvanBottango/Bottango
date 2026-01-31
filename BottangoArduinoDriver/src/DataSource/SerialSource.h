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

	void resetBuffer() override;

private:
	/**
	 * @brief Checks the hash of a command string for validity.
	 * @param cmdString Pointer to the command string to check.
	 * @return true if the hash is valid, false otherwise.
	 */
	bool checkHash(const char* cmdString);

	unsigned long timeOfLastChar = 0;
	bool commandInProgress = false;

	char serialCommandBuffer[MAX_COMMAND_LENGTH];
	int serialCommandIdx = 0;
};

#endif