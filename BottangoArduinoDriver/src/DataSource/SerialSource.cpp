#include "SerialSource.h"
#include "../Modules/Outgoing.h"
#include "../BasicCommands.h"

void SerialSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}
	readData();
}

void SerialSource::init()
{
	resetBuffer();

	Serial.begin(BAUD_RATE);

	Outgoing::printLine();
	Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
	Outgoing::printLine();
}

void SerialSource::readData()
{
	while (Serial.available() > 0)
	{
		processData(Serial.read());
	}

	checkTimeout();
}