#include "MACResponder.h"
#ifdef RELAY_SUPPORTED

#include "../../Services/PersistentConfigUtil.h"
#include "../../Communication/Outgoing.h"

void MACResponder::onMultiMessageStart()
{}

bool MACResponder::emitNextChunk()
{
	if (hasEmittedAny())
	{
		return false;
	}

	char buffer[15];
	PersistentConfigUtil::getThisDeviceMacAddress(buffer);
	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(true);
	}

	Outgoing::printOutputStringPROGMEM(REPLY_MAC_ADDRESS);
	Outgoing::printOutputStringFlash(F(","));
	Outgoing::printOutputStringMem(buffer);
	Outgoing::printLine();

	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(false);
	}

	return true;
}

void MACResponder::cleanUpMultiMessage()
{
	delete this;
}

#endif