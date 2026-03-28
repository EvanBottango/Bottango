#pragma once

#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "../../Module Handling/ModuleLoop.h"

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
	virtual void initializeAsBridge() = 0;
	virtual void registerPeer(const uint8_t* udid) = 0;
	virtual void deregisterPeer(const uint8_t* udid) = 0;

	// Peer
	virtual void initializeAsPeer() = 0;
	virtual void peerPrint(const char* str) = 0;
	virtual void peerPrint(const __FlashStringHelper* str) = 0;
	virtual void peerPrintln() = 0;
	virtual void peerFlush() = 0;
	virtual bool peerRecvAvailable() = 0;
	virtual char peerReadNextChar() = 0;

	RelayChildPool* getPeerPool() const { return _relayPool; }
	RelayRole getRole() const { return _relayRole; }	

	void setLastHeartbeatTime(unsigned long time) { lastHeartbeatTime = time; }

protected:
	RelayChildPool* _relayPool = nullptr;
	RelayRole _relayRole = RelayRole::None;

	unsigned long lastHeartbeatTime = 0;
};

#endif; // RELAY_SUPPORTED