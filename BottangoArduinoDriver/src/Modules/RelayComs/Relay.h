#pragma once

#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "../../Module Handling/LoopModule.h"

/*
Naming should move to:
relay - feature set especially shared elements
Bridge - thing that has peers
Peer - things that connect to bridge
*/

class Relay : public LoopModule
{
public:
	enum class RelayRole
	{
		Bridge,
		Peer,
		None
	};

	void init() override;

	// Bridge
	virtual void initializeAsBridge() {}
	virtual void registerPeer(const uint8_t* udid) {}
	virtual void deregisterPeer(const uint8_t* udid) {}
	virtual bool stop(bool doUninitialize);

	// Peer
	virtual void initializeAsPeer() {}
	virtual void peerPrint(const char* str) {}
	virtual void peerPrint(const __FlashStringHelper* str) {}
	virtual void peerPrintln() {}
	virtual void peerFlush() {}
	virtual bool peerRecvAvailable() { return false; }
	virtual char peerReadNextChar() { return '\0'; }

	RelayChildPool* getPeerPool() const { return _relayPool; }

	RelayRole getRole() const { return _relayRole; }
	bool isBridge() const { return _relayRole == RelayRole::Bridge; }
	bool isPeer() const { return _relayRole == RelayRole::Peer; }

	void setLastHeartbeatTime(unsigned long time) { lastHeartbeatTime = time; }

protected:
	RelayChildPool* _relayPool = nullptr;
	RelayRole _relayRole = RelayRole::None;

	unsigned long lastHeartbeatTime = 0;
};

#endif // RELAY_SUPPORTED