#include "I2SAudEventStatusResponder.h"
#if defined(AUDIO_SD_I2S)

#include "Modules/Outgoing.h"
#include "DataSource/SDCardUtil.h"
#include "../BottangoArduinoConfig.h"

I2SAudEventStatusResponder::I2SAudEventStatusResponder(byte incomingStatus)
{
	status = incomingStatus;
}

void I2SAudEventStatusResponder::onMultiMessageStart()
{}

bool I2SAudEventStatusResponder::emitNextChunk()
{
	if (hasEmittedAny())
	{
		return false;
	}

	_outgoing->printOutputStringPROGMEM(REPLY_AUD_STATUS);
	_outgoing->printOutputStringFlash(F(","));
	_outgoing->printOutputStringMem(status);
	_outgoing->printLine();

	return true;
}

void I2SAudEventStatusResponder::cleanUpMultiMessage()
{
	delete this;
}

#endif