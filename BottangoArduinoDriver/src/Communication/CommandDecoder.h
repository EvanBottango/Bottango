// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "../DataSource/DataSource.h"
#include "../Module Handling/ModuleLoop.h"

class CommandDecoder : public LoopModule
{
public:
	struct splitCommandData
	{
		char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
		char* stringToSplit;
#ifdef ALLOW_SYNC_COMMANDS
		//char* currentCommand = nullptr;
		char* syncCommandToSplit = nullptr;
		char* commandEnd = nullptr;
		char* currentFrameStart = nullptr;
		char* nextFrameStart = nullptr;
		bool expectNewCommand = true;
		bool syncCommandInProgress = false;
#endif // ALLOW_SYNC_COMMANDS
	};

	/**
	 * @brief Decode data from the data source.
	 */
	virtual void decode() {};

	/**
	 * @brief Returns true if there is a valid command available to consume.
	 * @return true if a command is available, false otherwise.
	 */
	virtual bool hasCommand();

	/**
	 * @brief Try to consume a command from the decoder.
	 * @return Pointer to the command parameters array if a command is available, nullptr otherwise.
	 */
	virtual char** tryConsumeCommand();

	/**
	 * @brief Set the data source for the command decoder.
	 * @param src Pointer to the data source.
	 */
	virtual void setDataSource(DataSource* src);

	/**
	 * @brief Splits a command string into components in-place.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the command to split. Must be non-null; the function will modify the buffer.
	 */
	virtual bool splitCommand(splitCommandData* data) const
	{
		return false;
	}
#ifdef ALLOW_SYNC_COMMANDS
	/**
	 * @brief Initializes the sync command parsing state with a new command string.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the sync command to parse. Must be non-null; the function will modify the buffer.
	 */
	virtual void beginSyncCommand(splitCommandData* data) const {};

	/**
	 * @brief Retrieves the next frame from the current sync command.
	 * @return Pointer to the next frame string, or nullptr if no more frames are available.
	 */
	virtual void getNextFrame(splitCommandData* data) const {};

	/**
	 * @brief Checks whether additional frames from a multi-frame are available.
	 * @return true if there are more frames available; false otherwise.
	 */
	virtual bool hasMoreFrames(splitCommandData* data) const
	{
		return false;
	}
#endif // ALLOW_SYNC_COMMANDS

protected:
	bool validCommandAvailable = false;
	DataSource* source;
};

#endif