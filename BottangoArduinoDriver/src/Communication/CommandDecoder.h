// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "DataSource.h"
#include "../Module Handling/ModuleLoop.h"

class CommandDecoder : public LoopModule
{
public:
	virtual void decode() {};

	virtual bool hasCommand();

	virtual bool tryConsumeCommand(char** out);

	virtual void setDataSource(DataSource* src);

protected:
	char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
	bool validFrameAvailable = false;
	DataSource* source;
};

#endif