// RS485.h

#ifndef _RS485_h
#define _RS485_h

#include <Arduino.h>
#include "DataSource.h"

// Note: This is a placeholder implementation.
// For not its only used to make the ModuleSlot approach more clear.

class RS485Source : public DataSource
{
public:


private:
	unsigned long timeOfLastChar = 0;
	bool commandInProgress = false;

	char serialCommandBuffer[MAX_COMMAND_LENGTH];
	int serialCommandIdx = 0;
};

#endif // _RS485_h