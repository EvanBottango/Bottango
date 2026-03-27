#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChildPool_h
#define RelayChildPool_h

#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"
#include "AbstractMultiMessageOutgoingSource.h"
#include "RelayChildMessageQueue.h"

#ifdef ESP32
#include <freertos/semphr.h>
#endif

// Forward declaration of RelayChild
class RelayChild;

const char RELAY_ID_RESPONSE_PREFIX[] PROGMEM = "rlyId,";

class RelayChildPool : public AbstractMultiMessageOutgoingSource
{

public:
	RelayChildPool();

#ifdef ESP32
	// lock / unlock relay pool access (task-safe)
	// Required because ESP-NOW task and main loop both access relay list/lifetime.
	// Keep lock holds short to avoid stalling the main loop.
	inline void lockPool()
	{
		configASSERT(_relayMutex);
		xSemaphoreTakeRecursive(_relayMutex, portMAX_DELAY);
	}

	inline void unlockPool()
	{
		configASSERT(_relayMutex);
		xSemaphoreGiveRecursive(_relayMutex);
	}
#else
	inline void lockPool() {}
	inline void unlockPool() {}
#endif

	void addRelay(RelayChild* relay);

	void removeRelay(int id);

	void passThroughCommandToRelay(int id, char** commands, byte paramsCount);

	void deregisterAll();

	void update();

	bool bridgeIsConnectedToAllPeers();

	void resumeTimeConnectedPeers(bool clearCurves);
	void stopTimeOnConnectedPeers();

	void sendHandshakeCommand(RelayChild* peer);

	void clearCurvesOnConnectedPeers();

	RelayChild* getRelay(int id);
	RelayChild* getRelay(const uint8_t* mac_addr);
	int getIdForRelay(RelayChild* relayChild);

	bool toPeerQueueFull() const { return _toPeerQueue.full(); }

	virtual void cleanUpMultiMessage() override;

	void setRelayIdToReport(int id);

	bool enqueueUnicastToPeerQueue(RelayChild* peer, char* commandString);

	RelayChildMessageQueue& outgoingQueue() { return _toPeerQueue; }

	void getConnectedRelayIds(int* outIds, uint8_t& outCount);
	void getUnconnectedRelayIds(int* outIds, uint8_t& outCount);
	void markPeerTx(int peerId);
	void markPeerPollOutstanding(int peerId);
	void reportLostPeer(int peerId);

private:
	RelayChildMessageQueue _toPeerQueue;
	int _relayIdToReport = -1;
	int _nextRelayId = 0;
	unsigned long _lastPollEnqueueTime = 0;
	unsigned long _lastBootEnqueueTime = 0;

	CircularArray<RelayChild> _relays = CircularArray<RelayChild>(MAX_RELAY_CHILD);

#ifdef ESP32
	SemaphoreHandle_t _relayMutex = nullptr;
#endif

	bool isMacEqual(const uint8_t* mac1, const uint8_t* mac2);
	int hash(const char* str);
	int allocateRelayId();

	bool buildPassThroughCommand(char* outBuffer, const char* commandString);
	void executePassThrough(RelayChild* peer, char* commandString);
	bool enqueueBroadcastPassThrough(char* commandString, MessageIntent intent, TargetGroup target);
	void enqueuePollBroadcast();
	void enqueueBootBroadcast();

	bool emitNextChunk() override;
};

#endif
#endif