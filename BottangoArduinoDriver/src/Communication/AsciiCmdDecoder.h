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
	void init() override;	
	char** tryConsumeCommand() override;

	/**
	 * @brief Splits a command string into components in-place.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the command to split. Must be non-null; the function will modify the buffer.
	 */
	bool splitCommand(SplitCommandData* data) const override;

#ifdef ALLOW_SYNC_COMMANDS
	/**
	 * @brief Initializes the sync command parsing state with a new command string.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the sync command to parse. Must be non-null; the function will modify the buffer.
	 */
	void beginSyncCommand(SplitCommandData* data) const override;

	/**
	 * @brief Retrieves the next frame from the current sync command.
	 * @return Pointer to the next frame string, or nullptr if no more frames are available.
	 */
	void getNextFrame(SplitCommandData* data) const override;

	/**
	 * @brief Checks whether additional frames from a multi-frame are available.
	 * @return true if there are more frames available; false otherwise.
	 */
	bool hasMoreFrames(SplitCommandData* data) const override;
#endif // ALLOW_SYNC_COMMANDS

private:
	void decode() override;

#ifdef RELAY_SUPPORTED
	bool splitRelayCommand(SplitCommandData* data) const;
#endif
};

#endif