// AsciiParser.h

#ifndef _AsciiParser_h
#define _AsciiParser_h

#include <Arduino.h>
#include "CommandDecoder.h"
#include "../Module Handling/ModuleLoop.h"

class Parser : public LoopModule
{
public:
	void onPhase(Phase p) override;

	void setCommandDecoder(CommandDecoder* cmdDecoder);
	
	bool parseCommand(char** splitCommandBuffer);

private:
	CommandDecoder* decoder;
};

#endif