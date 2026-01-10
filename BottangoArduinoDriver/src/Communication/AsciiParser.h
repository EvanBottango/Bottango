// AsciiParser.h

#ifndef _AsciiParser_h
#define _AsciiParser_h

#include <Arduino.h>
#include "CmdParser.h"

class AsciiParser : public CmdParser
{
public:
	bool parseCommand(DataSource* source) override;

private:
	bool splitIntoBuffer(char* stringToSplit, byte& paramsCount);
};

#endif

