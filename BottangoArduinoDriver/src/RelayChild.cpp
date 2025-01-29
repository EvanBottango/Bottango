#include "../BottangoArduinoModules.h"
#if defined(RELAY_PARENT)
#include "RelayChild.h"
#include "ESPNOWUtil.h"
#include "BasicCommands.h"
#include "Errors.h"

const char RELAY_PASS_UP[] PROGMEM = "uR,";

RelayChild::RelayChild(char *identifier, char *macAddress)
{
    strcpy(this->identifier, identifier);

    // convert c string mac address to uint8_t[]
    for (int i = 0; i < 6; i++)
    {
        char byteStr[3] = {macAddress[2 * i], macAddress[2 * i + 1], '\0'};
        this->mac_addr[i] = (uint8_t)strtol(byteStr, NULL, 16);
    }

    ESPNowUtil::registerPeer(this->mac_addr);
}

void RelayChild::getIdentifier(char *outArray, short arraySize)
{
    strcpy(outArray, identifier);
}

void RelayChild::passDownCommands(char *commands)
{
    if (toChildBuffer.isFull())
    {
        // todo error here
        return;
    }
    toChildBuffer.addTxt(commands);
}

void RelayChild::passUpCommands(char *commands)
{
    block = true;
    if (toParentBuffer.isFull())
    {
        // todo error here
        return;
    }
    toParentBuffer.addTxt(commands);
    block = false;
}

void RelayChild::update()
{
    // any pass up to parent?
    if (!block && toParentBuffer.available())
    {
        // get message out of the buffer
        char message[MAX_COMMAND_LENGTH];
        toParentBuffer.getNextTxt(message);

        // pass up if needed
        bool passUp = false;

        // ok recieved, tx complete, but no need to pass on
        // unless finishing handshake
        if (strcmp(message, "OK\n") == 0)
        {
            if (passUpNextOK)
            {
                passUp = true;
                passUpNextOK = false;
            }
        }
        // String starts with "btngoHSK"
        // handshake response, needs to pass on
        // and send the next OK up
        else if (strncmp_P(message, BasicCommands::HANDSHAKE, 8) == 0)
        {
            passUp = true;
            passUpNextOK = true;
        }

        if (passUp)
        {
            // add pass up command, and identifier, then pass up
            char passUpCommand[MAX_COMMAND_LENGTH];
            // add command
            strcpy_P(passUpCommand, RELAY_PASS_UP);
            // add ID
            strcat(passUpCommand, identifier);
            // add ","
            strcat(passUpCommand, BottangoCore::delimiters);
            // add recv commands
            strcat(passUpCommand, message);

            // double check last char is newline (and add if not)
            size_t length = strlen(passUpCommand);
            if (length == 0)
            {
                passUpCommand[0] = '\n';
                passUpCommand[1] = '\0';
            }
            else if (passUpCommand[length - 1] != '\n' && length < MAX_COMMAND_LENGTH)
            {
                passUpCommand[length] = '\n';
                passUpCommand[length + 1] = '\0';
            }
            Outgoing::printOutputStringMem(passUpCommand);
        }
    }

    // any pass down to child?
    if (toChildBuffer.available())
    {
        char message[MAX_COMMAND_LENGTH];
        toChildBuffer.getNextTxt(message);

        ESPNowUtil::sendCommand(mac_addr, message);
    }
}

void RelayChild::destroy()
{
    ESPNowUtil::deregisterPeer(this->mac_addr);
}
#endif