// AsciiParser.h

#ifndef _AsciiParser_h
#define _AsciiParser_h

#include <Arduino.h>
#include "FrameDecoder.h"

class Parser
{
public:
	bool parseCommand(FrameDecoder* decoder);

private:
	bool splitIntoBuffer(char* stringToSplit, byte& paramsCount);
};

#endif