#include "I2SAudEventStatusResponder.h"
#if defined(AUDIO_SD_I2S)

#include "../Communication/Outgoing.h"
#include "../DataSource/SDCardUtil.h"
#include "../../BottangoArduinoConfig.h"

I2SAudEventStatusResponder::I2SAudEventStatusResponder(byte incomingStatus)
{
	status = incomingStatus;
}

void I2SAudEventStatusResponder::onMultiMessageStart()
{
}

bool I2SAudEventStatusResponder::emitNextChunk()
{
	if (hasEmittedAny())
	{
		return false;
	}

#ifdef RELAY_SUPPORTED
	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(true);
	}
#endif
	Outgoing::printOutputStringPROGMEM(REPLY_AUD_STATUS);
	Outgoing::printOutputStringFlash(F(","));
	Outgoing::printOutputStringMem(status);
	Outgoing::printLine();

#ifdef RELAY_SUPPORTED
	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(false);
	}
#endif

	return true;
}

void I2SAudEventStatusResponder::cleanUpMultiMessage()
{
	delete this;
}

#endif
