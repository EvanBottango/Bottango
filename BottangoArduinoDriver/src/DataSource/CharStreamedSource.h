#pragma once

#include <Arduino.h>
#include "DataSource.h"

/**
 * @brief Char streamed data source. Commands are expected to be terminated with a newline character ('\n') and include a hash for validation.
 */
class CharStreamedSource : public DataSource
{
public:
	bool tryConsumeData(char** out) override;
	void resetBuffer() override;

protected:
	void processData(char const incomingChar);
	void checkTimeout();

private:
	/**
	 * @brief Checks the hash of a command string for validity.
	 * @param cmdString Pointer to the command string to check.
	 * @return true if the hash is valid, false otherwise.
	 */
	static bool checkHash(const char* cmdString);

	/**
	 * @brief Stores the time of the last received character for timeout handling.
	 */
	unsigned long _timeOfLastChar = 0;

	/**
	 * @brief Helper, that indicates whether a command is currently being received. Used for timeout handling.
	 */
	bool _commandInProgress = false;

	/** @brief The buffer for a command. This buffer is used for all following phases: Decode, Parse and Execute, until the command is consumed.
	 * It is not copied, we work directly with this buffer
	 */
	char _serialCommandBuffer[MAX_COMMAND_LENGTH] = {};

	/**
	 * @brief Helper, to point to the current position in the buffer for storing incoming characters.
	 */
	int _serialCommandIdx = 0;
};