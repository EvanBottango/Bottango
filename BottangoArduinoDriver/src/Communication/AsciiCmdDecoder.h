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
	void splitCommand(char* stringToSplit);

#ifdef ALLOW_SYNC_COMMANDS
	char* buffer = nullptr;
	char* currentCommand = nullptr;
	char* commandEnd = nullptr;
	char* currentFrameStart = nullptr;
	char* nextFrameStart = nullptr;
	bool expectNewCommand = true;
	bool syncCommandInProgress = false;

	void beginSyncCommand(char* stringToSplit);

	char* getNextFrame();

	bool hasMoreFrames();
#endif
};

#endif

