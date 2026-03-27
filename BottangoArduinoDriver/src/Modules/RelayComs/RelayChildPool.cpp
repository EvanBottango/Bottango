#include "../BottangoArduinoModules.h"

#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "RelayChild.h"
#include "BasicCommands.h"
#include <limits.h>
#ifdef TOGGLE_DEBUG
#include "PersistentConfigUtil.h"
#endif

#if defined(RELAY_COMS_ESPNOW)
#define RELAY_RESPONSE_TIMEOUT RELAY_ESPNOW_RESPONSE_TIMEOUT
#elif defined(RELAY_COMS_RS485)
#define RELAY_RESPONSE_TIMEOUT RELAY_RS485_RESPONSE_TIMEOUT
#endif

RelayChildPool::RelayChildPool()
{
#ifdef ESP32
	_relayMutex = xSemaphoreCreateRecursiveMutex();
	configASSERT(_relayMutex);
#endif
}

void RelayChildPool::addRelay(RelayChild* relay)
{
	lockPool();

	if (_relays.size() >= MAX_RELAY_CHILD)
	{
		Error::reportError_NoSpaceAvailable(); // todo unique out of space
		unlockPool();
		return;
	}

	RelayChild* existingRelay = getRelay(relay->mac_addr);

	if (existingRelay != nullptr)
	{
		Error::reportError_RelayCollision(relay->mac_addr);
		unlockPool();
		return;
	}
	else
	{
		int relayId = allocateRelayId();
		if (relayId < 0)
		{
			unlockPool();
			return;
		}
		relay->stableId = relayId;
		_relays.pushBack(relay);
	}

	unlockPool();
}

void RelayChildPool::removeRelay(int id)
{
	lockPool();

	RelayChild* relay = getRelay(id);
	if (relay == nullptr)
	{
		Error::reportError_NoRelayForID(id);
		unlockPool();
		return;
	}

	_relays.remove(relay);
	relay->destroy();
	delete relay;

	unlockPool();
}

void RelayChildPool::passThroughCommandToRelay(int id, char** commands, byte paramsCount)
{
	lockPool();

	RelayChild* relay = getRelay(id);
	if (relay == nullptr)
	{
		Error::reportError_NoRelayForID(id);
		unlockPool();
		return;
	}

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';

	if (paramsCount != 4)
	{
		// always should come in the exact same format, 4 total params
		// todo error here
		unlockPool();
		return;
	}

	// skip 0 - relay command
	// skip 1 - relay identifier
	// use 2 - all commands to pass through
	// skip 3 - previous hash

	// add all commands to pass
	strcat(commandBuffer, commands[2]);

	executePassThrough(relay, commandBuffer);

	unlockPool();
}

void RelayChildPool::deregisterAll()
{
	lockPool();

	for (int i = 0; i < _relays.size(); i++)
	{
		RelayChild* relay = _relays.get(i);
		relay->destroy();
		delete _relays.get(i);
	}
	_relays.clear();

	unlockPool();
}

RelayChild* RelayChildPool::getRelay(const uint8_t* mac_addr)
{
	lockPool();
	RelayChild* found = nullptr;
	for (byte i = 0; i < _relays.size(); i++)
	{
		RelayChild* iterator = _relays.get(i);
		if (isMacEqual(mac_addr, iterator->mac_addr))
		{
			found = iterator;
			break;
		}
	}
	unlockPool();
	return found;
}

RelayChild* RelayChildPool::getRelay(int id)
{
	lockPool();
	if (id < 0)
	{
		unlockPool();
		return nullptr;
	}
	RelayChild* found = nullptr;
	for (int i = 0; i < _relays.size(); i++)
	{
		RelayChild* iterator = _relays.get(i);
		if (iterator != nullptr && iterator->stableId == id)
		{
			found = iterator;
			break;
		}
	}
	unlockPool();
	return found;
}

bool RelayChildPool::isMacEqual(const uint8_t* mac1, const uint8_t* mac2)
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
	unsigned long now = millis();
	RelayChild* snapshot[MAX_RELAY_CHILD] = {};
	int snapshotCount = 0;

	lockPool();
	int relayCount = _relays.size();
	for (int i = 0; i < relayCount && i < MAX_RELAY_CHILD; i++)
	{
		snapshot[snapshotCount] = _relays.get(i);
		snapshotCount++;
	}
	unlockPool();

	for (int i = 0; i < snapshotCount; i++)
	{
		RelayChild* iterator = snapshot[i];
		if (iterator == nullptr)
		{
			continue;
		}
		iterator->update();

		if (iterator->pollOutstandingAndExpired(now, RELAY_RESPONSE_TIMEOUT))
		{
			// TODO decide how to handle a missing poll response
			iterator->clearPollOutstanding();
			reportLostPeer(iterator->stableId);
		}
	}

	if (now - _lastPollEnqueueTime >= RELAY_POLL_INTERVAL_AS_BRIDGE && !_toPeerQueue.full())
	{
		int connectedCount = 0;
		for (int i = 0; i < snapshotCount; i++)
		{
			RelayChild* peer = snapshot[i];
			if (peer != nullptr && peer->connected)
			{
				connectedCount++;
			}
		}
		if (connectedCount > 0)
		{
			enqueuePollBroadcast();
			_lastPollEnqueueTime = now;
		}
	}

	if (now - _lastBootEnqueueTime >= RELAY_BOOT_INTERVAL_AS_BRIDGE && !_toPeerQueue.full())
	{
		int unconnectedCount = 0;
		for (int i = 0; i < snapshotCount; i++)
		{
			RelayChild* peer = snapshot[i];
			if (peer != nullptr && !peer->connected)
			{
				unconnectedCount++;
			}
		}
		if (unconnectedCount > 0)
		{
			enqueueBootBroadcast();
			_lastBootEnqueueTime = now;
		}
	}
}

int RelayChildPool::hash(const char* str)
{
	int finalValue = 0;
	while (*str != '\0')
	{
		finalValue += static_cast<unsigned char>(*str);
		++str;
	}
	return finalValue;
}

int RelayChildPool::allocateRelayId()
{
	if (_nextRelayId >= INT_MAX)
	{
		Error::reportError_NoSpaceAvailable();
		return -1;
	}
	return _nextRelayId++;
}

int RelayChildPool::getIdForRelay(RelayChild* relayChild)
{
	lockPool();
	int id = relayChild ? relayChild->stableId : -1;
	unlockPool();
	return id;
}

bool RelayChildPool::emitNextChunk()
{
	if (_relayIdToReport < 0 || hasEmittedAny())
	{
		return false;
	}

	Outgoing::printOutputStringPROGMEM(RELAY_ID_RESPONSE_PREFIX); // rlyId,
	Outgoing::printOutputStringMem(_relayIdToReport);              // rlyId,2
	Outgoing::printLine();
	_relayIdToReport = -1;
	return true;
}

void RelayChildPool::cleanUpMultiMessage()
{
	_relayIdToReport = -1;
}

void RelayChildPool::setRelayIdToReport(int id)
{
	_relayIdToReport = id;
}

bool RelayChildPool::bridgeIsConnectedToAllPeers()
{
	lockPool();

	for (byte i = 0; i < _relays.size(); i++)
	{
		RelayChild* iterator = _relays.get(i);
		if (!iterator->connected)
		{
			unlockPool();
			return false;
		}
	}
	unlockPool();
	return true;
}

void RelayChildPool::stopTimeOnConnectedPeers()
{
	lockPool();

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';
	strcat(commandBuffer, BasicCommands::TIME_SYNC);
	strcat(commandBuffer, BottangoCore::delimiters);
	strcat(commandBuffer, BasicCommands::RELAY_PEER_STOP_TIME);

	enqueueBroadcastPassThrough(commandBuffer, MessageIntent::Normal, TargetGroup::BroadcastConnected);

	unlockPool();
}

void RelayChildPool::clearCurvesOnConnectedPeers()
{
	lockPool();

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';
	strcat(commandBuffer, BasicCommands::CLEAR_ALL_CURVES);

	enqueueBroadcastPassThrough(commandBuffer, MessageIntent::Normal, TargetGroup::BroadcastConnected);

	unlockPool();
}

void RelayChildPool::sendHandshakeCommand(RelayChild* peer)
{
	lockPool();

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';
	strcat(commandBuffer, BasicCommands::HANDSHAKE_REQUEST);
	strcat(commandBuffer, BottangoCore::delimiters);
	strcat(commandBuffer, "0");

	executePassThrough(peer, commandBuffer);

	unlockPool();
}

bool RelayChildPool::buildPassThroughCommand(char* outBuffer, const char* commandString)
{
	outBuffer[0] = '\0';
	strcat(outBuffer, commandString);

	// generate hash up to last token, which is the hash
	char hashBuffer[12];
	int hashResult = hash(outBuffer);
	itoa(hashResult, hashBuffer, 10);

	int used = (int)strlen(outBuffer);
	int hashLen = (int)strlen(hashBuffer);
	int totalLen = used + 2 + hashLen + 1; // ",h" + hash + "\n"
	if (totalLen >= MAX_COMMAND_LENGTH)
	{
		Error::reportError_CmdTooLong(totalLen);
		return false;
	}

	// add hash
	strcat(outBuffer, ",h");
	strcat(outBuffer, hashBuffer);

	// finish with NL
	strcat(outBuffer, "\n");

	return true;
}

void RelayChildPool::executePassThrough(RelayChild* peer, char* commandString)
{
	char commandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(commandBuffer, commandString))
	{
		return;
	}

	peer->passDownCommands(commandBuffer);
}

bool RelayChildPool::enqueueBroadcastPassThrough(char* commandString, MessageIntent intent, TargetGroup target)
{
	char commandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(commandBuffer, commandString))
	{
		return false;
	}

	return _toPeerQueue.enqueueMessage(-1, commandBuffer, intent, target);
}

void RelayChildPool::enqueuePollBroadcast()
{
	if (_toPeerQueue.full())
	{
		return;
	}

	char payloadBuffer[MAX_COMMAND_LENGTH];
	payloadBuffer[0] = '\0';
	strcat(payloadBuffer, BasicCommands::RELAY_POLL_REQUEST);

	char commandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(commandBuffer, payloadBuffer))
	{
		return;
	}

	enqueueBroadcastPassThrough(payloadBuffer, MessageIntent::Poll, TargetGroup::BroadcastConnected);
}

void RelayChildPool::enqueueBootBroadcast()
{
	if (_toPeerQueue.full())
	{
		return;
	}

	char payloadBuffer[MAX_COMMAND_LENGTH];
	payloadBuffer[0] = '\0';
	strcat(payloadBuffer, BasicCommands::REQUEST_PEER_BOOT);

	char commandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(commandBuffer, payloadBuffer))
	{
		return;
	}

	enqueueBroadcastPassThrough(payloadBuffer, MessageIntent::Boot, TargetGroup::BroadcastUnconnected);
}

bool RelayChildPool::enqueueUnicastToPeerQueue(RelayChild* peer, char* commandString)
{
	lockPool();

	if (_toPeerQueue.full())
	{
		unlockPool();
		return false;
	}

	bool enqueued = _toPeerQueue.enqueueMessage(getIdForRelay(peer), commandString, MessageIntent::Normal, TargetGroup::Unicast);
	unlockPool();
	return enqueued;
}

void RelayChildPool::resumeTimeConnectedPeers(bool clearCurves)
{
	lockPool();

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';

	if (clearCurves)
	{
		strcat(commandBuffer, BasicCommands::CLEAR_ALL_CURVES);
		enqueueBroadcastPassThrough(commandBuffer, MessageIntent::Normal, TargetGroup::BroadcastConnected);
		commandBuffer[0] = '\0';
	}

	strcat(commandBuffer, BasicCommands::TIME_SYNC);
	strcat(commandBuffer, BottangoCore::delimiters);
	strcat(commandBuffer, "0");
	enqueueBroadcastPassThrough(commandBuffer, MessageIntent::Normal, TargetGroup::BroadcastConnected);

	unlockPool();
}

void RelayChildPool::getConnectedRelayIds(int* outIds, uint8_t& outCount)
{
	lockPool();

	outCount = 0;
	for (int i = 0; i < _relays.size(); i++)
	{
		RelayChild* peer = _relays.get(i);
		if (peer != nullptr && peer->connected)
		{
			outIds[outCount] = peer->stableId;
			outCount++;
			if (outCount >= MAX_RELAY_CHILD)
			{
				unlockPool();
				return;
			}
		}
	}
	unlockPool();
}

void RelayChildPool::getUnconnectedRelayIds(int* outIds, uint8_t& outCount)
{
	lockPool();

	outCount = 0;
	for (int i = 0; i < _relays.size(); i++)
	{
		RelayChild* peer = _relays.get(i);
		if (peer != nullptr && !peer->connected)
		{
			outIds[outCount] = peer->stableId;
			outCount++;
			if (outCount >= MAX_RELAY_CHILD)
			{
				unlockPool();
				return;
			}
		}
	}
	unlockPool();
}

void RelayChildPool::markPeerTx(int peerId)
{
	lockPool();

	RelayChild* peer = getRelay(peerId);
	if (peer != nullptr)
	{
		peer->markTxTime();
	}

	unlockPool();
}

void RelayChildPool::markPeerPollOutstanding(int peerId)
{
	lockPool();

	RelayChild* peer = getRelay(peerId);
	if (peer != nullptr)
	{
		peer->markPollOutstanding();
	}

	unlockPool();
}

void RelayChildPool::reportLostPeer(int peerId)
{
	Outgoing::printOutputStringFlash(F("ERR: Lost peer "));
	Outgoing::printOutputStringMem(peerId);
	Outgoing::printLine();

	Outgoing::printOutputStringPROGMEM(BasicCommands::LOST_PEER);
	Outgoing::printOutputStringMem(peerId);
	Outgoing::printLine();
}

#endif