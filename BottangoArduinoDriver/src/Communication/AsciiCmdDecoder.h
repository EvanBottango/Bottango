// AsciiFrameDecoder.h

#ifndef _AsciiFrameDecoder_h
#define _AsciiFrameDecoder_h

#include <Arduino.h>
#include "CommandDecoder.h"
#include "../../BottangoArduinoConfig.h"

class AsciiCmdDecoder : public CommandDecoder
{
public:
	void onPhase(Phase p) override;

	void decode() override;

	char** tryConsumeCommand() override;

private:
	/**
	 * @brief Splits a command string into components in-place.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the command to split. Must be non-null; the function will modify the buffer.
	 */
	void splitCommand(char* stringToSplit);

#ifdef ALLOW_SYNC_COMMANDS
	char* buffer = nullptr;
	char* currentCommand = nullptr;
	char* commandEnd = nullptr;
	char* currentFrameStart = nullptr;
	char* nextFrameStart = nullptr;
	bool expectNewCommand = true;
	bool syncCommandInProgress = false;

	/**
	 * @brief Initializes the sync command parsing state with a new command string.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the sync command to parse. Must be non-null; the function will modify the buffer.
	 */
	void beginSyncCommand(char* stringToSplit);

	/**
	 * @brief Retrieves the next frame from the current sync command.
	 * @return Pointer to the next frame string, or nullptr if no more frames are available.
	 */
	char* getNextFrame();

	/**
	 * @brief Checks whether additional frames from a multi-frame are available.
	 * @return true if there are more frames available; false otherwise.
	 */
	bool hasMoreFrames();
#endif
};

#endif

