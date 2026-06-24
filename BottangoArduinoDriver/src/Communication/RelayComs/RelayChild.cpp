#include "RelayChild.h"
#if defined(RELAY_SUPPORTED)

#include "../BasicCommands.h"
#include "../../Util/UDIDHelper.h"
#include "../../System/Errors.h"
#include "../../Services/ModulesResponder.h"
#include "../../../BottangoArduinoConfig.h"


const char RELAY_PASS_UP[] PROGMEM = "uR,";

RelayChild::RelayChild(char* macAddress)
{
	connected = false;
	(void)UDIDHelper::convertCStrToMAC(macAddress, this->mac_addr);

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

	BottangoCore::relayComs->registerPeer(this->mac_addr);
}

void RelayChild::onReboot()
{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		char buffer[20];
		UDIDHelper::convertMACToCStr(mac_addr, buffer);
		Outgoing::printOutputStringFlash(F("Reboot occured on "));
		Outgoing::printOutputStringMem(buffer);
		Outgoing::printLine();
	}
#endif
	connected = false;
	pollOutstanding = false;
}

void RelayChild::passDownCommands(char* commands, MessageIntent intent)
{
	// connected and to peer queue has space?
	// send away!
	bool enqueSucceded = false;
	if (connected && !BottangoCore::relayPool->toPeerQueueFull())
	{
		enqueSucceded = BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, commands, intent);
	}

	// otherwise...
	if (!enqueSucceded)
	{
		// something already in the holding buffer until ready to send?
		// if so, it needs to be tossed
		if (singleMessageHoldingBuffer[0] != '\0')
		{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::printOutputStringFlash(F("Toss prev in holding buffer "));
				Outgoing::printOutputStringMem(singleMessageHoldingBuffer);
				Outgoing::printLine();
			}

#endif
			singleMessageHoldingBuffer[0] = '\0';
			singleMessageHoldingIntent = MessageIntent::Normal;
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
		strcpy(singleMessageHoldingBuffer, commands);
		singleMessageHoldingIntent = intent;
	}
}

void RelayChild::passUpCommands(char* commands)
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
	// if not connected yet
	// check if we're ready to be connected
	if (!connected)
	{
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
					char macbuffer[20];
					UDIDHelper::convertMACToCStr(mac_addr, macbuffer);
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
		else if (strncmp_P(message, BasicCommands::RELAY_POLL_RESPONSE, 7) == 0)
		{
			pollOutstanding = false;
			return;
		}

		// don't output pass up if in export mode
		// unless extra logging
		if (BottangoCore::isOffline())
		{
			// handle requests from peer
			if (strncmp_P(message, Outgoing::REQ_SHUTDOWN, 7) == 0)
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

		int prefixLen = (int)sizeof(RELAY_PASS_UP) - 1; // "uR,"
		int idLen = (int)strlen(idStr);
		int msgLen = (int)strlen(message);
		int totalLen = prefixLen + idLen + 1 + msgLen; // + delimiter
		if (totalLen >= MAX_COMMAND_LENGTH)
		{
			Error::reportError_CmdTooLong(totalLen);
			return;
		}

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
			if (singleMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
			{
				bool enqueued = BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, singleMessageHoldingBuffer, singleMessageHoldingIntent);
				if (enqueued)
				{
					singleMessageHoldingBuffer[0] = '\0';
					singleMessageHoldingIntent = MessageIntent::Normal;
				}
			}
		}
	}
}

void RelayChild::destroy()
{
	BottangoCore::relayComs->deregisterPeer(this->mac_addr);
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
		UDIDHelper::convertMACToCStr(mac_addr, buffer);
		Outgoing::printOutputStringMem(buffer);
		Outgoing::printLine();
	}
#endif

	if (singleMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
	{
		bool enqueued = BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, singleMessageHoldingBuffer, singleMessageHoldingIntent);
		if (enqueued)
		{
			singleMessageHoldingBuffer[0] = '\0';
			singleMessageHoldingIntent = MessageIntent::Normal;
		}
	}

	connected = true;
	pollOutstanding = false;
}

void RelayChild::markTxTime()
{
	lastTxTime = millis();
}

void RelayChild::markPollOutstanding()
{
	pollOutstanding = true;
}

void RelayChild::clearPollOutstanding()
{
	pollOutstanding = false;
}

bool RelayChild::pollOutstandingAndExpired(unsigned long now, unsigned long timeout)
{
	return pollOutstanding && (now - lastTxTime > timeout);
}
#endif