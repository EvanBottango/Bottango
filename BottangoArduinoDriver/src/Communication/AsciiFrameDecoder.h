// AsciiFrameDecoder.h

#ifndef _AsciiFrameDecoder_h
#define _AsciiFrameDecoder_h

#include <Arduino.h>
#include "FrameDecoder.h"
#include "../../BottangoArduinoConfig.h"

class AsciiFrameDecoder : public FrameDecoder
{
public:
	void decode(DataSource* source) override;
};

#endif

