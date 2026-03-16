// AsciiParser.h

#ifndef _AsciiParser_h
#define _AsciiParser_h

#include <Arduino.h>
#include "CommandDecoder.h"
#include "../Module Handling/ModuleLoop.h"

/**
 * @brief The Parser class is responsible for parsing commands provided by the CommandDecoder and executing the corresponding actions.
 * It operates during the Logic phase of the main loop, consuming commands from the decoder and invoking the appropriate handlers based on the command name and parameters.
 */
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
	bool parseCommand(char** splitCommandBuffer) const;

	unsigned long getStartTime(char* command) const;

	unsigned long getEndTime(char* command) const;

	/**
	 * @brief Helper function to determine if a given command name corresponds to a command that has a start time parameter.
	 * @param commandName The name of the command to check.
	 * @return True if the command has a start time parameter, false otherwise.
	 */
	bool commandHasStartTime(char* commandName) const;

	/**
	 * @brief Helper function to determine if a given command name corresponds to a command that has an end time parameter.
	 * @param commandName The name of the command to check.
	 * @return True if the command has an end time parameter, false otherwise.
	 */
	bool commandHasEndTime(char* commandName) const;

private:
	CommandDecoder* _decoder;
};

#endif