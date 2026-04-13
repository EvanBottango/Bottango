#ifndef _Parser_h
#define _Parser_h

#include <Arduino.h>
#include "CommandDecoder.h"
#include "../Module Handling/LoopModule.h"

/**
 * @brief The Parser class is responsible for parsing commands provided by the CommandDecoder and executing the corresponding actions.
 * It operates during the Logic phase of the main loop, consuming commands from the decoder and invoking the appropriate handlers based on the command name and parameters.
 */
class Parser : public LoopModule
{
public:
	void onPhase(Phase const p) override;

	/**
	 * @brief Set the command decoder to use for parsing commands.
	 * @param cmdDecoder Pointer to the CommandDecoder instance.
	 */
	void setCommandDecoder(CommandDecoder* cmdDecoder);
	
	/**
	 * @brief Parse a command from the split command buffer.
	 * @param splitCommandBuffer Pointer to an array of strings representing the split command and its parameters.
	 * @param sourceIsUsbSerial Indicates whether the command originated from the USB serial interface
	 * @return true if the command was successfully parsed and executed, false otherwise.
	 */
	bool parseCommand(char** splitCommandBuffer, bool const sourceIsUsbSerial) const;

	/**
	 * @brief Helper to get the earliest start time from a command string. Will also find the earliest start time across all frames of a multi-frame sync command, if applicable.
	 * @Note: For Multi-Frame: The command with the earliest start time may not be the same as the command with the latest end time
	 * @param command Pointer to the command string to parse. Must be a writable, null-terminated string, The string will be modified during parsing.
	 * @return The earliest start time in milliseconds for the command, or the current time if no valid start time is found.
	 */
	unsigned long getStartTime(char* command) const;

	/**
	 * @brief Helper to get the latest end time from a command string. Will also find the latest end time across all frames of a multi-frame sync command, if applicable.
	 * @Note: For Multi-Frame: The command with the latest end time may not be the same as the command with the earliest start time
	 * @param command Pointer to the command string to parse. Must be a writable, null-terminated string, The string will be modified during parsing.
	 * @return The latest end time in milliseconds for the command, or the current time if no valid end time is found.
	 */
	unsigned long getEndTime(char* command) const;

	/**
	 * @brief Helper function to determine if a given command name corresponds to a command that has a start time parameter.
	 * @param commandName The name of the command to check.
	 * @return True if the command has a start time parameter, false otherwise.
	 */
	bool commandHasStartTime(char* const commandName) const;

	/**
	 * @brief Helper function to determine if a given command name corresponds to a command that has an end time parameter.
	 * @param commandName The name of the command to check.
	 * @return True if the command has an end time parameter, false otherwise.
	 */
	bool commandHasDuration(char* const commandName) const;

private:
	bool commandIsAllowed(char* const commandName, bool const sourceIsUsbSerial) const;

	CommandDecoder* _decoder = nullptr;
};

#endif