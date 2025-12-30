#include "../BottangoArduinoModules.h"

#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "RelayChild.h"
#include "BasicCommands.h"

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

    RelayChild *existingRelay = getRelay(relay->mac_addr);

    if (existingRelay != nullptr)
    {
        Error::reportError_RelayCollision(relay->mac_addr);
        return;
    }
    else
    {
        relays.pushBack(relay);
        if (messageStatus == sendID)
        {
            relayIdToReport = getIdForRelay(relay);
        }
    }
}

void RelayChildPool::removeRelay(int id)
{
    RelayChild *relay = BottangoCore::relayPool->getRelay(id);
    if (relay == nullptr)
    {
        Error::reportError_NoRelayForID(id);
        return;
    }

    relays.remove(relay);
    relay->destroy();
    delete relay;
}

void RelayChildPool::passThroughCommandToRelay(int id, char **commands, byte paramsCount)
{
    RelayChild *relay = BottangoCore::relayPool->getRelay(id);
    if (relay == nullptr)
    {
        Error::reportError_NoRelayForID(id);
        return;
    }

    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';

    if (paramsCount != 4)
    {
        // always should come in the exact same format, 4 total params
        // todo error here
        return;
    }

    // skip 0 - relay command
    // skip 1 - relay identifier
    // use 2 - all commands to pass through
    // skip 3 - previous hash

    // add all commands to pass
    strcat(commandBuffer, commands[2]);

    executePassThrough(relay, commandBuffer);
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

RelayChild *RelayChildPool::getRelay(int id)
{
    return relays.get(id);
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

int RelayChildPool::getIdForRelay(RelayChild *relayChild)
{
    for (int i = 0; i < relays.size(); i++)
    {
        if (relays.get(i) == relayChild)
        {
            return i;
        }
    }
    return -1;
}

void RelayChildPool::initializeMultiMessage()
{
    messageStatus = sendID;
}

bool RelayChildPool::multiMessageisComplete()
{
    return messageStatus == complete;
}

void RelayChildPool::updateMultiMessage()
{
    switch (messageStatus)
    {
    case idle:
        break;
    case sendID:
        Outgoing::printOutputStringPROGMEM(RELAY_ID_RESPONSE_PREFIX); // rlyId,
        Outgoing::printOutputStringMem(relayIdToReport);              // rlyId,2
        Outgoing::printLine();
        messageStatus = waitingForContinue;
        relayIdToReport = -1;
        break;
    case waitingForContinue:
        // if (!hasOutgoingMessage)
        // {
        //     messageStatus = complete;
        // }
        break;
    }
}

void RelayChildPool::cleanUpMultiMessage()
{
    messageStatus = idle;
    relayIdToReport = -1;
}

bool RelayChildPool::bridgeIsConnectedToAllPeers()
{
    for (byte i = 0; i < relays.size(); i++)
    {
        RelayChild *iterator = relays.get(i);
        if (!iterator->connected)
        {
            return false;
        }
    }
    return true;
}

void RelayChildPool::stopTimeOnConnectedPeers()
{
    for (int i = 0; i < relays.size(); i++)
    {
        RelayChild *iterator = relays.get(i);
        if (iterator->connected)
        {
            sendStopTimeCommand(iterator);
        }
    }
}

void RelayChildPool::sendStopTimeCommand(RelayChild *peer)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    strcat(commandBuffer, BasicCommands::TIME_SYNC);
    strcat(commandBuffer, BottangoCore::delimiters);
    strcat(commandBuffer, BasicCommands::RELAY_PEER_STOP_TIME);

    executePassThrough(peer, commandBuffer);
}

void RelayChildPool::sendHeartbeat(RelayChild *peer)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    strcat(commandBuffer, BasicCommands::RELAY_HEARTBEAT_REQUEST);

    executePassThrough(peer, commandBuffer);
}

void RelayChildPool::clearCurvesOnConnectedPeers()
{
    for (int i = 0; i < relays.size(); i++)
    {
        RelayChild *iterator = relays.get(i);
        if (iterator->connected)
        {
            sendClearCurvesCommand(iterator);
        }
    }
}

void RelayChildPool::sendClearCurvesCommand(RelayChild *peer)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    strcat(commandBuffer, BasicCommands::CLEAR_ALL_CURVES);

    executePassThrough(peer, commandBuffer);
}

void RelayChildPool::sendHandshakeCommand(RelayChild *peer)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    strcat(commandBuffer, BasicCommands::HANDSHAKE_REQUEST);
    strcat(commandBuffer, BottangoCore::delimiters);
    strcat(commandBuffer, "0");

    executePassThrough(peer, commandBuffer);
}

void RelayChildPool::executePassThrough(RelayChild *peer, char *commandString)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';
    strcat(commandBuffer, commandString);

    // generate hash up to last token, which is the hash
    char hashBuffer[12];
    int hashResult = hash(commandBuffer);

    // add hash
    strcat(commandBuffer, ",h");
    strcat(commandBuffer, itoa(hashResult, hashBuffer, 10));

    // finish with NL
    strcat(commandBuffer, "\n");

    peer->passDownCommands(commandBuffer);
}

bool RelayChildPool::enqueueToSendQueue(RelayChild *peer, char *commandString)
{
    if (toPeerQueue.full())
    {
        return false;
    }

    return toPeerQueue.enqueue(peer->mac_addr, commandString);
}

void RelayChildPool::resumeTimeConnectedPeers(bool clearCurves)
{
    char commandBuffer[MAX_COMMAND_LENGTH];
    commandBuffer[0] = '\0';

    if (clearCurves)
    {
        strcat(commandBuffer, BasicCommands::CLEAR_ALL_CURVES);

        for (int i = 0; i < relays.size(); i++)
        {
            RelayChild *peer = relays.get(i);
            if (peer->connected)
            {
                executePassThrough(peer, commandBuffer);
            }
        }
        commandBuffer[0] = '\0';
    }

    strcat(commandBuffer, BasicCommands::TIME_SYNC);
    strcat(commandBuffer, BottangoCore::delimiters);
    strcat(commandBuffer, "0");

    for (int i = 0; i < relays.size(); i++)
    {
        RelayChild *peer = relays.get(i);
        if (peer->connected)
        {
            executePassThrough(peer, commandBuffer);
        }
    }
}

#endif