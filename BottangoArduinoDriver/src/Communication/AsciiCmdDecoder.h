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
};

#endif

