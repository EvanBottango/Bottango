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

	virtual char** tryConsumeCommand();

	virtual void setDataSource(DataSource* src);

protected:
	char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
	bool validCommandAvailable = false;
	DataSource* source;
};

#endif