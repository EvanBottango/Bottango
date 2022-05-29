#include "BotTangoSerialCommand.h"

BotTangoSerialCommand::BotTangoSerialCommand()
{
	initialized = false;
}

BotTangoSerialCommand::BotTangoSerialCommand(const char *commandNameIn, void (*functionIn)(char *[]))
{
	initialized = true;
	commandName = commandNameIn;
	function = functionIn;
}

void BotTangoSerialCommand::excecute(char *args[])
{
	function(args);
}