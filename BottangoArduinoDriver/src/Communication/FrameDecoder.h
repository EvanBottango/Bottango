// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "DataSource.h"

class FrameDecoder
{
public:
	virtual void decode(DataSource* source) {};

	virtual bool hasFrame() {}

	virtual bool tryConsumeFrame(char** out) {}

protected:
	char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
	bool validFrameAvailable = false;
};

#endif