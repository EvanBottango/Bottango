#ifndef BotTangoSerialCommandParser_h
#define BotTangoSerialCommandParser_h

#include "BotTangoSerialCommand.h"
#include "../BottangoArduinoConfig.h"

class CommandRegistry
{
public:
    CommandRegistry();

    void addCommand(const char *, void (*function)(char *[]));

    void executeCommand(char *commandString);

    unsigned long getMSTimeOfCommand(char *commandString);

    bool externalCommandIsValid(char *commandString);

private:
    static BotTangoSerialCommand *findCommand(char *name);
};

#endif