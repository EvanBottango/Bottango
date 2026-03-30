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

	// ToDo: Hier muss ein passender switch rein, damit das an den richtigen Kanal gesendet wird
	/*if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(true);
	}*/

	_outgoing->printOutputStringPROGMEM(REPLY_MAC_ADDRESS);
	_outgoing->printOutputStringFlash(F(","));
	_outgoing->printOutputStringMem(buffer);
	_outgoing->printLine();

	/*if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(false);
	}*/

	return true;
}

void MACResponder::cleanUpMultiMessage()
{
	delete this;
}

#endif // RELAY_SUPPORTED