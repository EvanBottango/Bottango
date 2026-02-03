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

	/**
	 * @brief Set the command decoder to use for parsing commands.
	 * @param cmdDecoder Pointer to the CommandDecoder instance.
	 */
	void setCommandDecoder(CommandDecoder* cmdDecoder);
	
	/**
	 * @brief Parse a command from the split command buffer.
	 * @param splitCommandBuffer Pointer to an array of strings representing the split command and its parameters.
	 * @return true if the command was successfully parsed and executed, false otherwise.
	 */
	bool parseCommand(char** splitCommandBuffer);

	unsigned long getStartTime(char* command);

	unsigned long getEndTime(char* command);

	bool commandHasStartTime(char* commandName) const;

	bool commandHasEndTime(char* commandName) const;

private:
	CommandDecoder* decoder;
};

#endif