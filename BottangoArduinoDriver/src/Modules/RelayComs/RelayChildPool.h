#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChildPool_h
#define RelayChildPool_h

#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"
#include "Modules/Outgoing.h"
#include "AbstractMultiMessageOutgoingSource.h"
#include "RelayChildMessageQueue.h"

#ifdef ESP32
#include <freertos/semphr.h>
#endif // ESP32

// Forward declaration of RelayChild
class RelayChild;

inline const char RELAY_ID_RESPONSE_PREFIX[] PROGMEM = "rlyId,";

class RelayChildPool : public AbstractMultiMessageOutgoingSource
{

public:
	RelayChildPool();

#ifdef ESP32
	// lock / unlock relay pool access (task-safe)
	// Required because ESP-NOW task and main loop both access relay list/lifetime.
	// Keep lock holds short to avoid stalling the main loop.
	inline void lockPool() const
	{
		configASSERT(_relayMutex);
		xSemaphoreTakeRecursive(_relayMutex, portMAX_DELAY);
	}

	inline void unlockPool() const
	{
		configASSERT(_relayMutex);
		xSemaphoreGiveRecursive(_relayMutex);
	}
#else
	inline void lockPool() {}
	inline void unlockPool() {}
#endif // ESP32

	void addPeer(RelayChild* newPeer);

	void removePeer(int id);

	void passThroughCommandToPeer(int id, char** commands);

	void deregisterAll();

	void update();

	bool bridgeIsConnectedToAllPeers() const;

	void resumeTimeConnectedPeers(bool clearCurves);
	void stopTimeOnConnectedPeers();

	void sendHandshakeCommand(RelayChild* peer);

	void clearCurvesOnConnectedPeers();

	RelayChild* getPeer(int id) const;
	RelayChild* getPeer(const uint8_t* mac_addr) const;
	int getIdForPeer(RelayChild* relayChild) const;	

	void setPeerIdToReport(int id);

	bool enqueueUnicastToPeerQueue(RelayChild* peer, char* commandString);	

	void getConnectedPeerIds(int* outIds, uint8_t& outCount) const;
	void getUnconnectedPeerIds(int* outIds, uint8_t& outCount) const;
	void markPeerTx(int peerId);
	void markPeerPollOutstanding(int peerId);
	void reportLostPeer(int peerId);

	bool toPeerQueueFull() const { return _toPeerQueue.full(); }
	RelayChildMessageQueue& getOutgoingQueue() { return _toPeerQueue; }

	// ==== AbstractMultiMessageOutgoingSource implementation ====
	virtual void cleanUpMultiMessage() override;

private:
	RelayChildMessageQueue _toPeerQueue;
	int _relayIdToReport = -1;
	int _nextRelayId = 0;
	unsigned long _lastPollEnqueueTime = 0;
	unsigned long _lastBootEnqueueTime = 0;

	CircularArray<RelayChild> _peers = CircularArray<RelayChild>(MAX_RELAY_CHILD);

#ifdef ESP32
	SemaphoreHandle_t _relayMutex = nullptr;
#endif // ESP32

	bool isMacEqual(const uint8_t* mac1, const uint8_t* mac2) const;
	int hash(const char* str) const;
	int allocateRelayId();

	bool buildPassThroughCommand(char* outBuffer, const char* commandString) const;
	void executePassThrough(RelayChild* peer, char* commandString);
	bool enqueueBroadcastPassThrough(char* commandString, MessageIntent intent, TargetGroup target);
	void enqueuePollBroadcast();
	void enqueueBootBroadcast();

	// ==== AbstractMultiMessageOutgoingSource implementation ====
	bool emitNextChunk() override;
};

#endif
#endif