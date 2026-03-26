#include "EspNowSource.h"
#include "../Modules/RelayComs/ESPNOWUtil.h"

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
	while (ESPNowUtil::peerRecvAvailable())
	{
		processData(ESPNowUtil::peerReadNextChar());
	}

	checkTimeout();
}