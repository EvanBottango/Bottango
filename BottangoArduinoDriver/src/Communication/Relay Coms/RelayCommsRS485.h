#pragma once

#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_RS485)

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "IRelayComms.h"
#include "../../Util/TxtBuffer.h"
#include "../../System/BottangoCore.h"

class RelayCommsRS485 : public IRelayComms
{
public:
	// ---- IRelayComms  ----
	void initializeAsBridge() override;
	void registerPeer(const uint8_t* udid) override;
	void deregisterPeer(const uint8_t* udid) override;

	void initializeAsPeer() override;
	void peerPrint(const char* str) override;
	void peerPrint(const __FlashStringHelper* str) override;
	void peerPrintln() override;
	void peerFlush() override;
	bool peerRecvAvailable() override;
	char peerReadNextChar() override;

	virtual void update() override;
	bool getIsBridge() override;

private:
	// ---- RS485 specific ----
	void initConnection();
	TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>* recvBuffer = nullptr;

	struct BridgeState
	{
		// header processing
		int respondingPeerID = 0;            // after tx a request to peer from bridge, what id should we expect a response from
		MessageIntent respondingIntent = MessageIntent::Normal;
		unsigned long responseWaitStart = 0; // ms timestamp when bridge starts waiting for a peer response
		bool peerAckRecv = false;
		bool tossRX = false; // bridge should toss any RX until the next EOT

		// broadcasting state tracking
		int broadcastTargets[MAX_RELAY_CHILD] = {};
		uint8_t broadcastCount = 0;
		uint8_t broadcastIdx = 0;
	};

	struct PeerState
	{
		// outgoing buffer
		TxtBuffer<TXT_BUFFER_SIZE_TX_COMMS>* txBuffer = nullptr;

		// header state tracking
		bool tossRX = false; // peer should toss any RX until the next EOT
		bool responsePending = false;
		bool txHeaderSent = false;
		unsigned long rxLastByteTime = 0;

		// outgoing flush flag
		bool flushToBridge = false; // peer should flush all TX in the buffer

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
		uint16_t lineBufferIdx = 0;
		char lineBuffer[MAX_COMMAND_LENGTH] = {};
#endif
	};

	BridgeState* bridgeState = nullptr;
	PeerState* peerState = nullptr;

	char peerIdBuffer[26] = {}; // char buffer for storing peer id for a header (MAC + comma + int max + null if reg) or just intMax + null if normal command
	byte peerIdBufferIDX = 0;

	bool isBridge = false;
	bool block = false;

	enum class LineStatus
	{
		idle,        // nothing to send, not waiting for anything in specific
		rxInHeader,  // readint header, need STX to exit
		rxInPayload, // reading payload, need an EoT to exit
		txOut,       // actively transmitting, send an EoT to exit
		fault        // something has gone wrong
	};

	LineStatus lineStatus = LineStatus::idle;
	void setLineStatus(LineStatus next);
	bool broadcastActive();
	bool tryPrepareNextOutgoing(OutgoingMessage& msg);
	void handleBridgeResponseTimeout();
	bool hasRoleState();
	bool parsePeerIdStrict(const char* text, int& outValue);

	// registration example flow:

	// bridge is **idle**
	// bridge pool:outgoingQueue has message
	// // message is a boot request

	// bridge is **tx**
	// bridge >> "/ENQmacmacmac,0/EOT
	// // bridge flushes
	// bridge is **rx** waiting on peer 0

	// peer A is **idle** and initialized
	// peer recv
	// "ENQ/macmacmac,0/EOT
	// and tosses it, as it's already init

	// peer B is **idle** and !initialized
	// peer recv
	// "ENQ/macmacmac,0/EOT
	// and tosses it as the mac doesn't match

	// peer c is **idle** and !initialized
	// peer recv
	// "ENQ/macmacmac,0/EOT
	// and takes it as the mac matches
	// peer c goes to tx

	// peer c tx message with
	// >> "ACK0/STX/OK\n/EOT"
	// peer c goes back to idle

	// peer a and b toss anything that starts with ack up to the next EoT

	// bridge recv "ACK0/STX/OK\n/EOT"
	// bridge checks that 0 is the peer expecting a response
	// bridge handles the response from the peer
	// bridge is **idle**

	// command example flow:

	// bridge is **idle**
	// bridge pool:outgoingQueue has message
	// // message is a cmd

	// bridge is **tx**
	// bridge >> "/SOH0/SOTxC,h1444\n/EOT"
	// // bridge flushes
	// bridge is **rx** waiting on peer 0

	// peer A and B are **idle** and initialized
	// peer recv
	// "/SOH0/SOTxC,h1444\n/EOT"
	// and tosses it, as it's not a header peer id match

	// peer C is **idle** and initialized
	// peer recv
	// "/SOH0/SOTxC,h1444\n/EOT"
	// and uses it as it's an id match

	// peer c tx message with
	// >> "ACK0/STX/OK\n/EOT"
	// peer c goes back to idle

	// peer a and b toss anything that starts with ack up to the next EoT

	// bridge recv "ACK0/STX/OK\n/EOT"
	// bridge checks that 0 is the peer expecting a response
	// bridge handles the response from the peer
	// bridge is **idle**

	///////////

	// • ENQ(0x05) — start of boot / registration request header
	// Ex Bridge → Peer: boot / registration request:
	// ENQ <mac-as-ascii> ',' <peerId-as-decimal> STX <payload> EOT

	// • SOH(0x01) — start of normal command request header
	// Ex Bridge → Peer: normal command request
	// SOH <peerId-as-decimal> STX <payload> EOT

	// • ACK(0x06) — start of peer response header(RS485_PEERACK)
	// Ex Peer → Bridge: response
	// ACK <peerId-as-decimal> STX <payload> EOT

	// • STX(0x02) — marks end of header / start of payload
	// • EOT(0x04) — marks end of frame / end of transmission
};

#endif // RELAY_COMS_RS485
