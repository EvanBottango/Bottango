// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "DataSource.h"

class CmdParser
{
	virtual bool parseCommand(DataSource* source) {};
};

#endif

