#include "../BottangoArduinoModules.h"
#if defined(RELAY_PARENT)

#include "RelayChildPool.h"
#include "RelayChild.h"

RelayChildPool::RelayChildPool()
{
}

void RelayChildPool::addRelay(RelayChild *relay)
{
    if (relays.size() >= MAX_RELAY_CHILD)
    {
        Error::reportError_NoSpaceAvailable(); // todo unique out of space
        return;
    }

    char relayIdentifier[9];
    relay->getIdentifier(relayIdentifier, 9);

    RelayChild *existingRelay = getRelay(relayIdentifier);

    if (relayIdentifier == nullptr)
    {
        Error::reportError_ServoCollision(); // todo unique collision
        return;
    }
    else
    {
        relays.pushBack(relay);
    }
}

void RelayChildPool::removeRelay(char *identifier)
{
    RelayChild *relay = getRelay(identifier);
    if (relay == nullptr)
    {
        Error::reportError_NoServoOnPin(); // todo unique error
        return;
    }

    relays.remove(relay);
    relay->destroy();
    delete relay;
}

void RelayChildPool::passThroughCommandToRelay(char *identifier, char **commands, byte commandCount)
{
    RelayChild *relay = getRelay(identifier);
    if (relay == nullptr)
    {
        Error::reportError_NoServoOnPin(); // todo unique error
        return;
    }

    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    // skip 0 - relay command
    // skip 1 - relay identifier
    // skip end - previous hash
    for (int i = 2; i < commandCount - 1; i++)
    {
        strcat(commandBuffer, commands[i]);
        if (i < commandCount - 2)
        {
            strcat(commandBuffer, ",");
        }
    }
    // generate hash up to last token, which is the hash
    char hashBuffer[12];
    int hashResult = hash(commandBuffer);

    // add hash
    strcat(commandBuffer, ",h");
    strcat(commandBuffer, itoa(hashResult, hashBuffer, 10));

    // finish with NL
    strcat(commandBuffer, "\n");

    relay->passDownCommands(commandBuffer);
}

void RelayChildPool::deregisterAll()
{
    for (int i = 0; i < relays.size(); i++)
    {
        RelayChild *relay = relays.get(i);
        relay->destroy();
        delete relays.get(i);
    }
    relays.clear();
}

RelayChild *RelayChildPool::getRelay(char *identifier)
{
    for (byte i = 0; i < relays.size(); i++)
    {
        char identifierBuffer[9];
        relays.get(i)->getIdentifier(identifierBuffer, 9);

        if (strcmp(identifier, identifierBuffer) == 0)
        {
            return relays.get(i);
        }
    }
    return nullptr;
}

RelayChild *RelayChildPool::getRelay(const uint8_t *mac_addr)
{
    for (byte i = 0; i < relays.size(); i++)
    {
        RelayChild *iterator = relays.get(i);
        if (isMacEqual(mac_addr, iterator->mac_addr))
        {
            return iterator;
        }
    }
    return nullptr;
}

bool RelayChildPool::isMacEqual(const uint8_t *mac1, const uint8_t *mac2)
{
    for (int i = 0; i < 6; i++)
    {
        if (mac1[i] != mac2[i])
        {
            return false;
        }
    }
    return true;
}

void RelayChildPool::update()
{
    for (byte i = 0; i < relays.size(); i++)
    {
        RelayChild *iterator = relays.get(i);
        iterator->update();
    }
}

int RelayChildPool::hash(const char *str)
{
    int finalValue = 0;
    while (*str != '\0')
    {
        finalValue += static_cast<unsigned char>(*str);
        ++str;
    }
    return finalValue;
}
#endif