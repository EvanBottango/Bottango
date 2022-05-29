#include "Arduino.h"
#include "CommandRegistry.h"
#include "CircularArray.h"
#include "BasicCommands.h"
#include "Time.h"
#include "../BottangoArduinoConfig.h"

// list of all possible commands, limited to the buffer size
BotTangoSerialCommand commands[COMMANDS_BUFFER_SIZE];

// default constructor
CommandRegistry::CommandRegistry() = default;

// given a name, and a function that can take an array of strings, add the command to the list of possible commands
void CommandRegistry::addCommand(const char *inCommand, void (*function)(char *[]))
{
    // find the first uninitialized command in the array
    for (int i = 0; i < COMMANDS_BUFFER_SIZE; i++)
    {
        if (!commands[i].initialized)
        {
            // set that command to the name and function given
            commands[i] = BotTangoSerialCommand(inCommand, function);
            return;
        }
    }

    // this is reached if nothing is assigned, because all the entries in the commands array have already been initialized
    Error::reportError_TooManyCommands();
}

char *splitCommandBuffer[COMMANDS_PARAMS_SIZE];

bool splitIntoBuffer(char *stringToSplit)
{
    byte idxResult = 0;
    char *wordStart;
    char delimiters[] = ",";

    if (idxResult + 1 >= COMMANDS_PARAMS_SIZE)
    {
        Error::reportError_TooManyParams();
        return false;
    }

    wordStart = strtok(stringToSplit, delimiters);
    while (wordStart != NULL)
    {
        splitCommandBuffer[idxResult++] = wordStart;
        wordStart = strtok(NULL, delimiters);
    }

    splitCommandBuffer[idxResult] = ((char *)"\0");

    return true;
}

// parses the serial buffer and turns it into commands
// param commandString: e.g. "rSP,9,1000,3000"
void CommandRegistry::executeCommand(char *commandString)
{
    bool splitSuccess = splitIntoBuffer(commandString);
    if (!splitSuccess)
    {
        return;
    }

    // The command name is the first string in the array, subsequent strings are parameters of that command
    char *commandName = splitCommandBuffer[0];

    BotTangoSerialCommand *command = findCommand(commandName);
    if (command == NULL)
    {
        ;
    }
    else
    {
        command->excecute(splitCommandBuffer);
    }
}

BotTangoSerialCommand *CommandRegistry::findCommand(char *commandName)
{
    // find the command (if any) that has the same command name as that first entry in the array of strings.
    for (int i = 0; i < COMMANDS_BUFFER_SIZE; i++)
    {
        // if a match is found
        if (commands[i].initialized && strcmp_P(commandName, commands[i].commandName) == 0)
        {
            return &commands[i];
        }
    }
    return NULL;
}

unsigned long CommandRegistry::getMSTimeOfCommand(char *commandString)
{
    unsigned long time = Time::getCurrentTimeInMs();
    bool splitSuccess = splitIntoBuffer(commandString);
    if (!splitSuccess)
    {
        return 0;
    }

    char *commandName = splitCommandBuffer[0];

    // only curves have time different than now, so parse the string to find the time of each curve type
    if (strcmp_P(commandName, BasicCommands::SET_CURVE) == 0)
    {
        time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
    }
    else if (strcmp_P(commandName, BasicCommands::SET_ONOFFCURVE) == 0)
    {
        time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
    }
    else if (strcmp_P(commandName, BasicCommands::SET_TRIGGERCURVE) == 0)
    {
        time = Time::getLastSyncedTimeInMs() + atol(splitCommandBuffer[2]);
    }

    return time;
}

bool CommandRegistry::externalCommandIsValid(char *commandString)
{
#ifndef USE_COMMAND_STRING
    return true;
#else
    bool splitSuccess = splitIntoBuffer(commandString);
    if (!splitSuccess)
    {
        return false;
    }

    char *commandName = splitCommandBuffer[0];

    // only handshake request is allowed
    if (strcmp_P(commandName, BasicCommands::HANDSHAKE_REQUEST) == 0)
    {
        return true;
    }

    return false;
#endif
}
