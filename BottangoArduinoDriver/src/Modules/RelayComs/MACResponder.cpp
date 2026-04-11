#include "MACResponder.h"
#ifdef RELAY_SUPPORTED
#include "PersistentConfigUtil.h"
#include "Modules/Outgoing.h"

void MACResponder::onMultiMessageStart()
{
}

bool MACResponder::emitNextChunk()
{
	if (hasEmittedAny())
	{
		return false;
	}

	char buffer[15];
	PersistentConfigUtil::getThisDeviceMacAddress(buffer);

	_outgoing->printOutputStringPROGMEM(REPLY_MAC_ADDRESS);
	_outgoing->printOutputStringFlash(F(","));
	_outgoing->printOutputStringMem(buffer);
	_outgoing->printLine();

	return true;
}

void MACResponder::cleanUpMultiMessage()
{
	delete this;
}

#endif // RELAY_SUPPORTED