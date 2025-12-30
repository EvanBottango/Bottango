#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)
#include "RelayChild.h"
#include "ESPNOWUtil.h"
#include "BasicCommands.h"
#include "Errors.h"
#include "ModulesResponder.h"
#include "../BottangoArduinoConfig.h"

const char RELAY_PASS_UP[] PROGMEM = "uR,";

RelayChild::RelayChild(char *macAddress)
{
    connected = false;
    ESPNowUtil::convertCStrToMac(macAddress, this->mac_addr);

    // Enque handshake into outgoing
    // if this bridge is in export anim mode
    if (BottangoCore::isOffline())
    {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled())
#endif
        {
            Outgoing::printOutputStringFlash(F("Exported Anim, queue handshake"));
            Outgoing::printLine();
        }

#endif
        BottangoCore::relayPool->sendHandshakeCommand(this);
    }

    ESPNowUtil::registerPeer(this->mac_addr);
}

void RelayChild::onReboot()
{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
    if (PersistentConfigUtil::debugEnabled())
#endif
    {
        char buffer[20];
        ESPNowUtil::convertMacToCStr(mac_addr, buffer);
        Outgoing::printOutputStringFlash(F("Reboot occured on "));
        Outgoing::printOutputStringMem(buffer);
        Outgoing::printLine();
    }
#endif
    connected = false;
}

void RelayChild::passDownCommands(char *commands)
{
    // connected and to peer queue has space?
    // send away!
    if (connected && !BottangoCore::relayPool->toPeerQueueFull())
    {
        BottangoCore::relayPool->enqueueToSendQueue(this, commands);
    }
    // otherwise...
    else
    {
        // something already in the holding buffer until ready to send?
        // if so, it needs to be tossed
        if (disconnectedMessageHoldingBuffer[0] != '\0')
        {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
            {
                Outgoing::printOutputStringFlash(F("Toss prev in holding buffer "));
                Outgoing::printOutputStringMem(disconnectedMessageHoldingBuffer);
                Outgoing::printLine();
            }

#endif
            disconnectedMessageHoldingBuffer[0] = '\0';
        }

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled())
#endif
        {
            Outgoing::printOutputStringFlash(F("Held for connection / space"));
            Outgoing::printLine();
        }

#endif

        // hold it until connected and space is available
        strcpy(disconnectedMessageHoldingBuffer, commands);
    }
}

void RelayChild::passUpCommands(char *commands)
{
    if (incomingFromPeerBuffer.isFull())
    {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
        if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
        {
            Outgoing::printOutputStringFlash(F("Pass up buffer full!"));
            Outgoing::printLine();
        }

#endif
        return;
    }
    incomingFromPeerBuffer.addTxt(commands);
}

void RelayChild::update()
{
    unsigned long now = millis();

#ifdef RELAY_LOGGING
    char macbuffer[20];
#ifdef TOGGLE_DEBUG
    if (PersistentConfigUtil::debugEnabled())
#endif
    {
        ESPNowUtil::convertMacToCStr(mac_addr, macbuffer);
    }
#endif

    // if not connected yet
    // check if we're ready to be connected
    if (!connected)
    {
        // have we recieved anything?
        if (now - lastMsgTime >= RELAY_BOOT_INTERVAL)
        {
            requestBoot();
        }

        while (incomingFromPeerBuffer.available())
        {
            // get message out of the buffer
            char message[MAX_COMMAND_LENGTH];
            incomingFromPeerBuffer.getNextTxt(message);

            // looking for starts with "BOOT"
            if (strncmp_P(message, BasicCommands::REPLY_PEER_BOOT, 5) == 0)
            {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
                if (PersistentConfigUtil::debugEnabled())
#endif
                {
                    Outgoing::printOutputStringFlash(F("Got peer boot response from peer: "));
                    Outgoing::printOutputStringMem(message);
                    Outgoing::printLine();
                }
#endif
                onConnectionComplete();
                break;
            }
#ifdef RELAY_LOGGING
            else
            {
                // otherwise toss it. Only want "BOOT"
#ifdef TOGGLE_DEBUG
                if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
                {
                    Outgoing::printOutputStringFlash(F("Toss non connected rcv on peer"));
                    Outgoing::printOutputStringMem(macbuffer);
                    Outgoing::printOutputStringFlash(F(": "));
                    if (strlen(message) > 0 && message[0] == '\n')
                    {
                        Outgoing::printOutputStringFlash(F("NL"));
                    }
                    else
                    {
                        Outgoing::printOutputStringMem(message);
                    }
                    Outgoing::printLine();
                }
            }
#endif
        }
    }

    // any pass up from peer to desktop as bridge?
    if (incomingFromPeerBuffer.available())
    {
        // get message out of the buffer
        char message[MAX_COMMAND_LENGTH];
        incomingFromPeerBuffer.getNextTxt(message);

        // Did we get a reboot "BOOT" message?
        if (strncmp_P(message, BasicCommands::BOOT, 4) == 0)
        {
            onReboot();
        }
        else if (strncmp_P(message, BasicCommands::RELAY_HEARTBEAT_RESPONSE, 7) == 0)
        {
            if (heartbeatOutstanding)
            {
                heartbeatOutstanding = false;
                return;
            }
        }

        // don't output pass up if in export mode
        // unless extra logging
        if (BottangoCore::isOffline())
        {
            // handle requests from peer
            if (strncmp_P(message, Outgoing::ESTOP, 7) == 0)
            {
                BottangoCore::stop(true);
            }
            else if (strncmp_P(message, Outgoing::STOP_PLAY, 8) == 0)
            {
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
                if (BottangoCore::commandStreamProvider != nullptr)
                {
                    BottangoCore::commandStreamProvider->stop();
                }
#endif
            }
#ifndef RELAY_LOGGING
            return;
#endif
        }

        int id = BottangoCore::relayPool->getIdForRelay(this);
        char idStr[10];
        itoa(id, idStr, 10);

        // add pass up command, and identifier, then pass up
        char passUpCommand[MAX_COMMAND_LENGTH];
        // add command
        strcpy_P(passUpCommand, RELAY_PASS_UP);
        // add ID
        strcat(passUpCommand, idStr);
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

        if (BottangoCore::isOffline())
        {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
            if (PersistentConfigUtil::debugEnabled())
#endif
            {
                Outgoing::printOutputStringFlash(F("Export toss pass Up: "));
                Outgoing::printOutputStringMem(passUpCommand);
            }

            return;
#endif
        }

        Outgoing::printOutputStringMem(passUpCommand);
    }

    // any pass down to child holding buffer if any
    if (connected)
    {
        if (!BottangoCore::relayPool->toPeerQueueFull())
        {
            if (!heartbeatOutstanding && now - lastMsgTime >= RELAY_HEARTBEAT_INTERVAL)
            {
                BottangoCore::relayPool->sendHeartbeat(this);
                lastMsgTime = now;
                heartbeatOutstanding = true;
            }
            if (disconnectedMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
            {
                BottangoCore::relayPool->enqueueToSendQueue(this, disconnectedMessageHoldingBuffer);
                disconnectedMessageHoldingBuffer[0] = '\0';
            }
        }

        // timeout on the heartbeat response
        if (heartbeatOutstanding && now - lastMsgTime > RELAY_HEARTBEAT_TIMEOUT)
        {
            Outgoing::printOutputStringPROGMEM(BasicCommands::LOST_PEER);
            Outgoing::printOutputStringMem(BottangoCore::relayPool->getIdForRelay(this));
            Outgoing::printLine();
            heartbeatOutstanding = false;
        }
    }
}

void RelayChild::destroy()
{
    ESPNowUtil::deregisterPeer(this->mac_addr);
}

void RelayChild::requestBoot()
{

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
    if (PersistentConfigUtil::debugEnabled())
#endif
    {
        Outgoing::printOutputStringFlash(F("Sending boot request to peer: "));
        char buffer[20];
        ESPNowUtil::convertMacToCStr(mac_addr, buffer);
        Outgoing::printOutputStringMem(buffer);
        Outgoing::printLine();
    }
#endif

    char message[MAX_COMMAND_LENGTH];
    message[0] = '\0';
    strcat(message, BasicCommands::REQUEST_PEER_BOOT);
    strcat(message, "\n");
    // direct to send, ignore connected state
    BottangoCore::relayPool->enqueueToSendQueue(this, message);
    lastMsgTime = millis();
}

void RelayChild::onConnectionComplete()
{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
    if (PersistentConfigUtil::debugEnabled())
#endif
    {
        Outgoing::printOutputStringFlash(F("Connected to peer: "));
        char buffer[20];
        ESPNowUtil::convertMacToCStr(mac_addr, buffer);
        Outgoing::printOutputStringMem(buffer);
        Outgoing::printLine();
    }
#endif

    if (disconnectedMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
    {
        BottangoCore::relayPool->enqueueToSendQueue(this, disconnectedMessageHoldingBuffer);
        disconnectedMessageHoldingBuffer[0] = '\0';
    }

    connected = true;
    lastMsgTime = millis();
}
#endif