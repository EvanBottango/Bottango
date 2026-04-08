#include "../BottangoArduinoModules.h"

#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "RelayChild.h"
#include "BasicCommands.h"
#include <limits.h>
#include "System/SystemStatus.h"
#ifdef TOGGLE_DEBUG
#include "PersistentConfigUtil.h"
#endif // TOGGLE_DEBUG

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

void RelayChildPool::addPeer(RelayChild* newPeer)
{
	lockPool();

	if (_peers.size() >= MAX_RELAY_CHILD)
	{
		Error::reportError_NoSpaceAvailable(); // todo unique out of space
		unlockPool();
		return;
	}

	RelayChild* existingPeer = getPeer(newPeer->mac_addr);

	if (existingPeer != nullptr)
	{
		Error::reportError_RelayCollision(newPeer->mac_addr);
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
		newPeer->stableId = relayId;
		_peers.pushBack(newPeer);
	}

	unlockPool();
}

void RelayChildPool::removePeer(int id)
{
	lockPool();

	RelayChild* relay = getPeer(id);
	if (relay == nullptr)
	{
		Error::reportError_NoRelayForID(id);
		unlockPool();
		return;
	}

	// enque a teardown message for the relay
	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';
	strcat(commandBuffer, BasicCommands::STOP);
	char passThroughCommandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(passThroughCommandBuffer, commandBuffer))
	{
		unlockPool();
		return;
	}

	// flag the peer as being torn down
	// that will stop all outgoing messages except stop
	// and it will eventually finalize teardown once stop is actually tx
	relay->teardown = true;
	relay->clearPollOutstanding();

	relay->passDownCommands(passThroughCommandBuffer, MessageIntent::Teardown);

	unlockPool();
}

void RelayChildPool::passThroughCommandToPeer(int id, char** commands)
{
	lockPool();

	RelayChild* relay = getPeer(id);
	if (relay == nullptr)
	{
		Error::reportError_NoRelayForID(id);
		unlockPool();
		return;
	}

	if (relay->teardown)
	{
		unlockPool();
		return;
	}

	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';

	//if (paramsCount != 4)
	//{
		// always should come in the exact same format, 4 total params
		// todo error here
		// ToDo: We can catch the "not 4 params" way earlier, without the need to pass a static number all the way to here
	//	unlockPool();
	//	return;
	//}

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

	for (int i = 0; i < _peers.size(); i++)
	{
		RelayChild* relay = _peers.get(i);
		relay->destroy();
		delete _peers.get(i);
	}
	_peers.clear();

	unlockPool();
}

RelayChild* RelayChildPool::getPeer(const uint8_t* mac_addr) const
{
	lockPool();
	RelayChild* found = nullptr;
	for (byte i = 0; i < _peers.size(); i++)
	{
		RelayChild* iterator = _peers.get(i);
		if (isMacEqual(mac_addr, iterator->mac_addr))
		{
			found = iterator;
			break;
		}
	}
	unlockPool();
	return found;
}

RelayChild* RelayChildPool::getPeer(int id) const
{
	lockPool();
	if (id < 0)
	{
		unlockPool();
		return nullptr;
	}
	RelayChild* found = nullptr;
	for (int i = 0; i < _peers.size(); i++)
	{
		RelayChild* iterator = _peers.get(i);
		if (iterator != nullptr && iterator->stableId == id)
		{
			found = iterator;
			break;
		}
	}
	unlockPool();
	return found;
}

bool RelayChildPool::isMacEqual(const uint8_t* mac1, const uint8_t* mac2) const
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

	// find and finalize teardown on any that are ready to complete teardown
	int pendingTeardownFinalizeIds[MAX_RELAY_CHILD] = {};
	int pendingTeardownFinalizeCount = 0;
	lockPool();
	for (int i = 0; i < _peers.size() && pendingTeardownFinalizeCount < MAX_RELAY_CHILD; i++)
	{
		RelayChild* peer = _peers.get(i);
		if (peer != nullptr && peer->teardownReadyToFinalize)
		{
			pendingTeardownFinalizeIds[pendingTeardownFinalizeCount] = peer->stableId;
			pendingTeardownFinalizeCount++;
		}
	}
	unlockPool();
	for (int i = 0; i < pendingTeardownFinalizeCount; i++)
	{
		finalizeRelayTeardown(pendingTeardownFinalizeIds[i]);
	}

	// snapshot the peers so we don't hold the mutex very long in everything that follows
	RelayChild* snapshot[MAX_RELAY_CHILD] = {};
	int snapshotCount = 0;
	lockPool();
	int peerCount = _peers.size();
	for (int i = 0; i < peerCount && i < MAX_RELAY_CHILD; i++)
	{
		snapshot[snapshotCount] = _peers.get(i);
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
			iterator->clearPollOutstanding();
			reportLostPeer(iterator->stableId);
		}
	}

	// Ping connected peers on a timer to prompt them to respond with "RLY_ACK", confirming they're still there and giving us an opportunity to detect lost peers that didn't cleanly disconnect
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

	// Bridge came active AFTER some peers already booted. To catch those peers, we can broadcast a boot message on a timer, prompting them to respond with "sBoot"
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

int RelayChildPool::hash(const char* str) const
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

int RelayChildPool::getIdForPeer(RelayChild* relayChild) const
{
	lockPool();
	int id = relayChild ? relayChild->stableId : -1;
	unlockPool();
	return id;
}

void RelayChildPool::setPeerIdToReport(int id)
{
	_relayIdToReport = id;
}

bool RelayChildPool::bridgeIsConnectedToAllPeers() const
{
	lockPool();

	for (byte i = 0; i < _peers.size(); i++)
	{
		RelayChild* iterator = _peers.get(i);
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

void RelayChildPool::beginPoolTeardown()
{
	if (isUninitializing)
	{
		return;
	}

	isUninitializing = true;

	lockPool();

	// clear the queue, everything is now stale except stop
	_toPeerQueue.clear();

	// all still-connected peers are now globally tearing down
	// clear out any unconnected peers
	RelayChild* unconnectedPeersToRemove[MAX_RELAY_CHILD] = {};
	int unconnectedPeersToRemoveCount = 0;

	// teardown connected, mark for destroy and remove unconneceted
	for (int i = 0; i < _peers.size(); i++)
	{
		RelayChild* peer = _peers.get(i);
		if (peer == nullptr)
		{
			continue;
		}

		if (peer->connected)
		{
			peer->teardown = true;
			peer->clearPollOutstanding();
		}
		else if (unconnectedPeersToRemoveCount < MAX_RELAY_CHILD)
		{
			unconnectedPeersToRemove[unconnectedPeersToRemoveCount] = peer;
			unconnectedPeersToRemoveCount++;
		}
	}

	// destroy unconnected
	for (int i = 0; i < unconnectedPeersToRemoveCount; i++)
	{
		RelayChild* peer = unconnectedPeersToRemove[i];
		_peers.remove(peer);
		peer->destroy();
		delete peer;
	}

	// enque teardown stop to all connected peers, including peers already marked teardown
	char commandBuffer[MAX_COMMAND_LENGTH];
	commandBuffer[0] = '\0';
	strcat(commandBuffer, BasicCommands::STOP);

	enqueueBroadcastPassThrough(commandBuffer, MessageIntent::Teardown, TargetGroup::BroadcastConnected);

	// lock additional message enqueue
	_toPeerQueue.lock();

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

bool RelayChildPool::buildPassThroughCommand(char* outBuffer, const char* commandString) const
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

	// ToDo: This buffer size seems excessive for a simple poll command. This call chain uses 3x MAX_COMMAND_LENGTH of RAM!
	char payloadBuffer[MAX_COMMAND_LENGTH];
	payloadBuffer[0] = '\0';
	strcat(payloadBuffer, BasicCommands::RELAY_POLL_REQUEST);

	char commandBuffer[MAX_COMMAND_LENGTH];
	if (!buildPassThroughCommand(commandBuffer, payloadBuffer))
	{
		return;
	}

	// ToDo: enqueueBroadcastPassThrough() packs the command AGAIN into [Command],h[Hash]\n format, which is unnecessary since buildPassThroughCommand already does this. Why?
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

bool RelayChildPool::enqueueUnicastToPeerQueue(RelayChild* peer, char* commandString, MessageIntent intent)
{
	lockPool();

	if (_toPeerQueue.full())
	{
		unlockPool();
		return false;
	}

	bool enqueued = _toPeerQueue.enqueueMessage(getIdForPeer(peer), commandString, intent, TargetGroup::Unicast);
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

void RelayChildPool::getConnectedPeerIds(int* outIds, uint8_t& outCount, bool includeTeardown) const
{
	lockPool();

	outCount = 0;
	for (int i = 0; i < _peers.size(); i++)
	{
		RelayChild* peer = _peers.get(i);
		if (peer != nullptr && peer->connected && (includeTeardown || !peer->teardown))
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

void RelayChildPool::getUnconnectedPeerIds(int* outIds, uint8_t& outCount) const
{
	lockPool();

	outCount = 0;
	for (int i = 0; i < _peers.size(); i++)
	{
		RelayChild* peer = _peers.get(i);
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

	RelayChild* peer = getPeer(peerId);
	if (peer != nullptr)
	{
		peer->markTxTime();
	}

	unlockPool();
}

void RelayChildPool::markPeerPollOutstanding(int peerId)
{
	lockPool();

	RelayChild* peer = getPeer(peerId);
	if (peer != nullptr)
	{
		if (peer->teardown)
		{
			unlockPool();
			return;
		}
		peer->markPollOutstanding();
	}

	unlockPool();
}

void RelayChildPool::markRelayTeardownReadyToFinalize(int peerId)
{
	lockPool();

	RelayChild* peer = getPeer(peerId);
	if (peer != nullptr)
	{
		peer->teardownReadyToFinalize = true;
	}

	unlockPool();
}

void RelayChildPool::finalizeRelayTeardown(int peerId)
{
	lockPool();

	RelayChild* peer = getPeer(peerId);
	if (peer == nullptr)
	{
		unlockPool();
		return;
	}

	_peers.remove(peer);
	peer->destroy();
	delete peer;

	unlockPool();
}

void RelayChildPool::reportLostPeer(int peerId)
{
	OutgoingSerial::printOutputStringFlash(F("ERR: Lost peer "));
	OutgoingSerial::printOutputStringMem(peerId);
	OutgoingSerial::printLine();

	OutgoingSerial::printOutputStringPROGMEM(BasicCommands::LOST_PEER);
	OutgoingSerial::printOutputStringMem(peerId);
	OutgoingSerial::printLine();

	//if (BottangoCore::isOffline())
	if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback)
	{
		Outgoing::printOutputStringFlash(F("Offline ERR: Teardown Bridge"));
		BottangoCore::request_eStop();
	}
}

// ==== AbstractMultiMessageOutgoingSource implementation ====
void RelayChildPool::cleanUpMultiMessage()
{
	_relayIdToReport = -1;
}

bool RelayChildPool::emitNextChunk()
{
	if (_relayIdToReport < 0 || hasEmittedAny())
	{
		return false;
	}

	_outgoing->printOutputStringPROGMEM(RELAY_ID_RESPONSE_PREFIX);		// rlyId,
	_outgoing->printOutputStringMem(_relayIdToReport);					// rlyId,2
	_outgoing->printLine();												// rlyId,2\n
	_relayIdToReport = -1;

	// ToDo: Bottango does not answer to rlyId with any OK\n.
	// Workaround: return false, this will end the multi-message response immediately after sending the rlyId
	return false;

	//return true;
}

#endif // RELAY_SUPPORTED