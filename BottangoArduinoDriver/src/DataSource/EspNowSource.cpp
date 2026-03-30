#include "EspNowSource.h"
#include "Modules/RelayComs/Relay.h"
#include "BottangoCore.h"

void EspNowSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}
	readData();
}

void EspNowSource::init()
{
	resetBuffer();
}

void EspNowSource::readData()
{
	Relay* relay = BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs);

	while (relay->peerRecvAvailable())
	{
		processData(relay->peerReadNextChar());
	}

	/*while (ESPNowUtil::peerRecvAvailable())
	{
		processData(ESPNowUtil::peerReadNextChar());
	}*/

	checkTimeout();
}