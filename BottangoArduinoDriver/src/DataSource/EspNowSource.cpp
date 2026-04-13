#include "EspNowSource.h"

#ifdef RELAY_COMS_ESPNOW
#include "Modules/RelayComs/Relay.h"
#include "BottangoCore.h"


void EspNowSource::onPhase(Phase const p)
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

	checkTimeout();
}
#endif // RELAY_COMS_ESPNOW