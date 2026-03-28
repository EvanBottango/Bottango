#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)
#include "RelayChild.h"
#include "BasicCommands.h"
#include "Errors.h"
#include "ModulesResponder.h"
#include "../BottangoArduinoConfig.h"
#include "UDIDHelper.h"
#include "System/SystemStatus.h"
#include "Module Handling/ModuleMaster.h"

const char RELAY_PASS_UP[] PROGMEM = "uR,";

RelayChild::RelayChild(char* macAddress)
{
	connected = false;
	(void)UDIDHelper::convertCStrToMAC(macAddress, this->mac_addr);

	// Enque handshake into outgoing
	// if this bridge is in export anim mode
	if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			OutgoingSerial::printOutputStringFlash(F("Exported Anim, queue handshake"));
			OutgoingSerial::printLine();
		}

#endif // RELAY_LOGGING
		//BottangoCore::relayPool->sendHandshakeCommand(this);
		BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->getPeerPool()->sendHandshakeCommand(this);
	}

	BottangoCore::mMaster.getModule<Relay>(Modules::RelayComs)->registerPeer(this->mac_addr);
}

void RelayChild::onReboot()
{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
	{
		char buffer[20];
		UDIDHelper::convertMACToCStr(mac_addr, buffer);
		OutgoingSerial::printOutputStringFlash(F("Reboot occured on "));
		OutgoingSerial::printOutputStringMem(buffer);
		OutgoingSerial::printLine();
	}
#endif // RELAY_LOGGING
	connected = false;
	_pollOutstanding = false;
}

void RelayChild::passDownCommands(char* commands)
{
	// connected and to peer queue has space?
	// send away!
	if (connected && !BottangoCore::relayPool->toPeerQueueFull())
	{
		BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, commands);
	}
	// otherwise...
	else
	{
		// something already in the holding buffer until ready to send?
		// if so, it needs to be tossed
		if (_disconnectedMessageHoldingBuffer[0] != '\0')
		{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::printOutputStringFlash(F("Toss prev in holding buffer "));
				Outgoing::printOutputStringMem(_disconnectedMessageHoldingBuffer);
				Outgoing::printLine();
			}

#endif
			_disconnectedMessageHoldingBuffer[0] = '\0';
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
		strcpy(_disconnectedMessageHoldingBuffer, commands);
	}
}

void RelayChild::passUpCommands(char* commands)
{
	if (_incomingFromPeerBuffer.isFull())
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
	_incomingFromPeerBuffer.addTxt(commands);
}

void RelayChild::update()
{
	// if not connected yet
	// check if we're ready to be connected
	if (!connected)
	{
		while (_incomingFromPeerBuffer.available())
		{
			// get message out of the buffer
			char message[MAX_COMMAND_LENGTH];
			_incomingFromPeerBuffer.getNextTxt(message);

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
	if (_incomingFromPeerBuffer.available())
	{
		// get message out of the buffer
		char message[MAX_COMMAND_LENGTH];
		_incomingFromPeerBuffer.getNextTxt(message);

		// Did we get a reboot "BOOT" message?
		if (strncmp_P(message, BasicCommands::BOOT, 4) == 0)
		{
			onReboot();
		}
		else if (strncmp_P(message, BasicCommands::RELAY_POLL_RESPONSE, 7) == 0)
		{
			_pollOutstanding = false;
			return;
		}

		// don't output pass up if in export mode
		// unless extra logging
		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
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

		if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
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
			if (_disconnectedMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
			{
				BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, _disconnectedMessageHoldingBuffer);
				_disconnectedMessageHoldingBuffer[0] = '\0';
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

	if (_disconnectedMessageHoldingBuffer[0] != '\0' && !BottangoCore::relayPool->toPeerQueueFull())
	{
		BottangoCore::relayPool->enqueueUnicastToPeerQueue(this, _disconnectedMessageHoldingBuffer);
		_disconnectedMessageHoldingBuffer[0] = '\0';
	}

	connected = true;
	_pollOutstanding = false;
}

void RelayChild::markTxTime()
{
	_lastTxTime = millis();
}

void RelayChild::markPollOutstanding()
{
	_pollOutstanding = true;
}

void RelayChild::clearPollOutstanding()
{
	_pollOutstanding = false;
}

bool RelayChild::pollOutstandingAndExpired(unsigned long now, unsigned long timeout)
{
	return _pollOutstanding && (now - _lastTxTime > timeout);
}
#endif