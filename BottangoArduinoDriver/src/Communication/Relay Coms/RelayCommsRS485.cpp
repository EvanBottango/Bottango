#include "RelayCommsRS485.h"
#if defined(RELAY_COMS_RS485)

#include <stdio.h>
#ifdef RELAY_LOGGING
#include <esp_err.h>
#endif
#include "RelayChild.h"
#include "../../Util/UDIDHelper.h"

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
static void logStringWithNL(const char* text)
{
	if (!text)
	{
		return;
	}

	for (size_t i = 0; text[i] != '\0'; i++)
	{
		if (text[i] == '\n')
		{
			Outgoing::printOutputStringFlash(F("NL"));
		}
		else
		{
			Outgoing::printOutputStringMem(text[i]);
		}
	}
}

static void logBytesWithNL(const uint8_t* data, uint16_t length)
{
	if (!data)
	{
		return;
	}

	for (uint16_t i = 0; i < length; i++)
	{
		unsigned char u = data[i];
		if (u == '\n')
		{
			Outgoing::printOutputStringFlash(F("NL"));
		}
		else if (u >= 32 && u <= 126)
		{
			Outgoing::printOutputStringMem(static_cast<char>(u));
		}
		else
		{
			Outgoing::printOutputStringMem(static_cast<char>(u));
		}
	}
}
#endif

void RelayCommsRS485::setLineStatus(LineStatus next)
{
	if (next == lineStatus)
	{
		return;
	}

	lineStatus = next;

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
	const __FlashStringHelper* statusString = F("idle");
	switch (next)
	{
	case LineStatus::rxInHeader:
		statusString = F("rxInHeader");
		break;
	case LineStatus::rxInPayload:
		statusString = F("rxInPayload");
		break;
	case LineStatus::txOut:
		statusString = F("txOut");
		break;
	case LineStatus::fault:
		statusString = F("fault");
		break;
	default:
		break;
	}

	if (isBridge)
	{
		Outgoing::printOutputStringFlash(F("Bridge state -> "));
		Outgoing::printOutputStringFlash(statusString);
		Outgoing::printLine();
	}
	else
	{
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("Peer state -> "));
		Outgoing::printOutputStringFlash(statusString);
		Outgoing::printLine();
		Outgoing::endToggleOnSecondaryOutgoing();
	}
#endif
}

// ensure a rcv peer id in a header is an actual int. Atoi will return 0 if fail, which is a valid id
bool RelayCommsRS485::parsePeerIdStrict(const char* text, int& outValue)
{
	if (text == nullptr || *text == '\0')
	{
		return false;
	}

	constexpr int kMaxPeerId = MAX_RELAY_CHILD - 1;
	int value = 0;

	for (const char* p = text; *p != '\0'; ++p)
	{
		const char c = *p;
		if (c < '0' || c > '9')
		{
			return false;
		}

		// ASCII digits are contiguous: '0' is 48, '1' is 49, ... '9' is 57.
		// Subtracting '0' converts the digit character to its numeric value.
		const int digit = c - '0';

		// Before doing: value = (value * 10) + digit
		// check that this would not exceed kMaxPeerId.
		// If value > (kMaxPeerId - digit) / 10, then value*10 + digit > kMaxPeerId.
		if (value > (kMaxPeerId - digit) / 10)
		{
			return false;
		}

		value = (value * 10) + digit;
	}

	outValue = value;
	return true;
}

bool RelayCommsRS485::broadcastActive()
{
	return bridgeState != nullptr && bridgeState->broadcastCount > 0;
}

bool RelayCommsRS485::tryPrepareNextOutgoing(OutgoingMessage& msg)
{
	if (bridgeState == nullptr)
	{
		return false;
	}

	while (BottangoCore::relayPool->outgoingQueue().peek(msg))
	{
		// toss stale unicast messages to peers that are tearing down
		if (msg.target == TargetGroup::Unicast)
		{
			RelayChild* peer = BottangoCore::relayPool->getRelay(msg.peerId);

			if (peer != nullptr && peer->teardown         // teardown message
				&& msg.intent != MessageIntent::Teardown) // but not a teardown intent
			{
				// skip it
				BottangoCore::relayPool->outgoingQueue().pop();
				continue;
			}
		}
		// broadcast
		else
		{
			// set up broadcast
			if (!broadcastActive())
			{
				// snapshot connected peers for this broadcast when it reaches the head of the queue
				if (msg.target == TargetGroup::BroadcastUnconnected)
				{
					BottangoCore::relayPool->getUnconnectedRelayIds(bridgeState->broadcastTargets, bridgeState->broadcastCount);
				}
				else
				{
					bool includeTeardown = msg.intent == MessageIntent::Teardown;
					BottangoCore::relayPool->getConnectedRelayIds(bridgeState->broadcastTargets, bridgeState->broadcastCount, includeTeardown);
				}
				bridgeState->broadcastIdx = 0;
			}

			// skip forward on peerIDX until we find a still active peer
			// needed in case a peer becomes inactive after enqueing a broadcast to it.
			while (broadcastActive() && bridgeState->broadcastIdx < bridgeState->broadcastCount)
			{
				int peerId = bridgeState->broadcastTargets[bridgeState->broadcastIdx];
				RelayChild* peer = BottangoCore::relayPool->getRelay(peerId);
				// skip null
				if (peer == nullptr)
				{
					bridgeState->broadcastIdx++;
					continue;
				}
				// stop advancing and use this peer if either:
				// - the peer is not tearing down and this is a normal message, OR
				// - this is itself a teardown broadcast, it goes to everyone
				if (!peer->teardown || msg.intent == MessageIntent::Teardown)
				{
					break;
				}

				bridgeState->broadcastIdx++;
			}

			if (!broadcastActive() || bridgeState->broadcastIdx >= bridgeState->broadcastCount)
			{
				// no valid targets remain in the snapshot, consume the broadcast entry for now
				bridgeState->broadcastIdx = 0;
				bridgeState->broadcastCount = 0;
				BottangoCore::relayPool->outgoingQueue().pop();
				continue;
			}
		}

		return true;
	}

	return false;
}

bool RelayCommsRS485::hasRoleState()
{
	return bridgeState != nullptr || peerState != nullptr;
}

// when a peer timesout on a bridge, handle and move on so we can keep the line alive.
void RelayCommsRS485::handleBridgeResponseTimeout()
{
	if (bridgeState == nullptr)
	{
		return;
	}

	RelayChild* peer = BottangoCore::relayPool->getRelay(bridgeState->respondingPeerID);

	// if it's a timed out teardown, mark it ready to finalize on the main loop and move on
	if (bridgeState->respondingIntent == MessageIntent::Teardown)
	{
		BottangoCore::relayPool->markRelayTeardownReadyToFinalize(bridgeState->respondingPeerID);
	}
	else if (peer != nullptr && peer->connected)
	{
		// Only report lost peer if it was actually connected in the first place
		BottangoCore::relayPool->reportLostPeer(bridgeState->respondingPeerID);
	}
	// otherwise, this was a timeout while still trying to connect the peer. Keep waiting.

	bridgeState->peerAckRecv = false;
	bridgeState->respondingIntent = MessageIntent::Normal;
	peerIdBufferIDX = 0;
	peerIdBuffer[0] = '\0';
	setLineStatus(LineStatus::idle);

	if (broadcastActive())
	{
		bridgeState->broadcastIdx++;
		if (bridgeState->broadcastIdx >= bridgeState->broadcastCount)
		{
			bridgeState->broadcastIdx = 0;
			bridgeState->broadcastCount = 0;
			BottangoCore::relayPool->outgoingQueue().pop();
		}
	}
}

void RelayCommsRS485::initializeAsBridge()
{
	if (hasRoleState())
	{
		return;
	}
	if (bridgeState == nullptr)
	{
		bridgeState = new BridgeState();
	}
	recvBuffer = new TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>();

	initConnection();

	isBridge = true;

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
	Outgoing::printOutputStringFlash(F("RS485 init as bridge"));
	Outgoing::printLine();
	Outgoing::printOutputStringFlash(F("RS485 pins: DE="));
	Outgoing::printOutputStringMem(RS485_DE_PIN);
	Outgoing::printOutputStringFlash(F(" RX="));
	Outgoing::printOutputStringMem(RS485_RX_PIN);
	Outgoing::printOutputStringFlash(F(" TX="));
	Outgoing::printOutputStringMem(RS485_TX_PIN);
	Outgoing::printLine();
#endif
}

void RelayCommsRS485::initializeAsPeer()
{
	if (hasRoleState())
	{
		return;
	}
	if (peerState == nullptr)
	{
		peerState = new PeerState();
	}

	recvBuffer = new TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>();
	peerState->txBuffer = new TxtBuffer<TXT_BUFFER_SIZE_TX_COMMS>();

	initConnection();

	isBridge = false;

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
	Outgoing::toggleOnSecondaryOutgoing();
	Outgoing::printOutputStringFlash(F("RS485 init as peer"));
	Outgoing::printLine();
	Outgoing::printOutputStringFlash(F("RS485 pins: DE="));
	Outgoing::printOutputStringMem(RS485_DE_PIN);
	Outgoing::printOutputStringFlash(F(" RX="));
	Outgoing::printOutputStringMem(RS485_RX_PIN);
	Outgoing::printOutputStringFlash(F(" TX="));
	Outgoing::printOutputStringMem(RS485_TX_PIN);
	Outgoing::printLine();
	Outgoing::endToggleOnSecondaryOutgoing();
#endif
}

void RelayCommsRS485::initConnection()
{
	pinMode(RS485_DE_PIN, OUTPUT);
	digitalWrite(RS485_DE_PIN, LOW); // rx

	RS485_SERIAL.begin(RS485_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
	lineStatus = LineStatus::idle;
}

// all tx and rx is handled in the update loop
// see header file for examples of how a frame is composed
void RelayCommsRS485::update()
{
	if (lineStatus == LineStatus::fault)
	{
		return;
	}
	// bridge
	if (isBridge)
	{
		if (bridgeState == nullptr)
		{
			return;
		}

		// check if a pending peer response has timed out
		if (lineStatus == LineStatus::rxInHeader || lineStatus == LineStatus::rxInPayload)
		{
			if (millis() - bridgeState->responseWaitStart > RELAY_RS485_RESPONSE_TIMEOUT)
			{
				handleBridgeResponseTimeout();
				return;
			}
		}

		// if idle, we should see nothing on the line, since we're the bridge
		if (lineStatus == LineStatus::idle)
		{
			if (RS485_SERIAL.available())
			{
				// this is an error case, there should be no rx on the bridge while idle
#ifdef RELAY_COMMS_LOGGING
				Outgoing::printOutputStringFlash(F("WARN: Bridge rx while idle"));
				Outgoing::printLine();
#endif
				while (RS485_SERIAL.available())
				{
					RS485_SERIAL.read();
				}
			}
		}

		// We're expecting a header from a peer, so read it
		if (lineStatus == LineStatus::rxInHeader)
		{
			// read chars into read buffer
			while (RS485_SERIAL.available())
			{
				char c = RS485_SERIAL.read();

				// first char should always be ack char from a peer response
				if (!bridgeState->peerAckRecv)
				{
					if (c != RS485_PEERACK)
					{
						// this is an error case, first peer char should always be ack
#ifdef RELAY_COMMS_LOGGING
						Outgoing::printOutputStringFlash(F("WARN: First char peer resp should be ack"));
						Outgoing::printLine();
#endif
						bridgeState->tossRX = true;
						bridgeState->peerAckRecv = false;
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
						setLineStatus(LineStatus::rxInPayload);
						break;
					}
					else
					{
						bridgeState->peerAckRecv = true;
					}
				}
				// reached end of header?
				else if (c == RS485_STX)
				{
					int peerId = 0;

					// peer id was not a number?
					if (!parsePeerIdStrict(peerIdBuffer, peerId))
					{
#ifdef RELAY_COMMS_LOGGING
						Outgoing::printOutputStringFlash(F("ERR: Bridge rx header invalid peer id"));
						Outgoing::printLine();
#endif
						bridgeState->tossRX = true;
						bridgeState->peerAckRecv = false;
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
						setLineStatus(LineStatus::rxInPayload);
						break;
					}
					// peer id matches the expected responder
					else if (peerId == bridgeState->respondingPeerID)
					{
						// matched responding peer
						setLineStatus(LineStatus::rxInPayload);
						bridgeState->peerAckRecv = false;
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
						break;
					}
					else
					{
						// this is an error case, wrong peer replied
#ifdef RELAY_COMMS_LOGGING
						Outgoing::printOutputStringFlash(F("WARN: Wrong peer replied"));
						Outgoing::printLine();
#endif
					}
				}
				// add char to the peer id buffer to parse the header
				else
				{
					if (peerIdBufferIDX >= (sizeof(peerIdBuffer) - 1))
					{
#ifdef RELAY_COMMS_LOGGING
						Outgoing::printOutputStringFlash(F("ERR: Bridge rx header overflow, tossing frame"));
						Outgoing::printLine();
#endif
						bridgeState->tossRX = true;
						bridgeState->peerAckRecv = false;
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
						setLineStatus(LineStatus::rxInPayload);
						break;
					}
					peerIdBuffer[peerIdBufferIDX] = c;
					peerIdBufferIDX++;
					peerIdBuffer[peerIdBufferIDX] = '\0';
				}
			}
		}

		// we got an expected header from a peer, and now we read the payload from the peer
		if (lineStatus == LineStatus::rxInPayload)
		{
			if (recvBuffer)
			{
				if (!bridgeState->tossRX && recvBuffer->isFull())
				{
					// this is an error case
					// buffer full, drop this frame and re-sync at EOT
#ifdef RELAY_COMMS_LOGGING
					Outgoing::printOutputStringFlash(F("ERR: Bridge rx buffer full, tossing frame"));
					Outgoing::printLine();
#endif
					bridgeState->tossRX = true;
					recvBuffer->clear();
				}

				bool eotRecv = false;
				// read chars into read buffer
				while (RS485_SERIAL.available())
				{
					char c = RS485_SERIAL.read();

					// got end of transmission from peer
					if (c == RS485_EOT)
					{
						eotRecv = true;
						break;
					}
					else
					{
						// due to header error, toss until we reach EoT
						if (bridgeState->tossRX)
						{
							continue;
						}

						// add char to the recv buffer
						recvBuffer->addChar(c);

						// got a full command if newline
						if (c == '\n')
						{
							// get responding relay for a message from the pool
							RelayChild* relay = BottangoCore::relayPool->getRelay(bridgeState->respondingPeerID);
							if (relay != nullptr)
							{
								// get message from buffer
								char message[MAX_COMMAND_LENGTH];
								recvBuffer->getNextTxt(message);

#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
								Outgoing::printOutputStringFlash(F("Bridge rx payload: "));
								logStringWithNL(message);
								Outgoing::printLine();
#endif

								// and pass it up to the relay
								relay->passUpCommands(message);
							}
							else
							{
								// Unknown relay: discard the buffered line and drop the rest of this frame.
								char message[MAX_COMMAND_LENGTH];
								recvBuffer->getNextTxt(message);
#ifdef RELAY_COMMS_LOGGING
								Outgoing::printOutputStringFlash(F("ERR: Bridge rx for unknown peer id"));
								Outgoing::printOutputStringMem(bridgeState->respondingPeerID);
								Outgoing::printLine();
#endif
								bridgeState->tossRX = true;
							}
						}
					}
				}

				if (eotRecv)
				{
					// Clear header error flag if we have one
					if (bridgeState->tossRX)
					{
						bridgeState->tossRX = false;
					}

					// set to idle
					setLineStatus(LineStatus::idle);

					// we closed the loop on the teardown message, so mark it ready to finalize on the main loop
					if (bridgeState->respondingIntent == MessageIntent::Teardown)
					{
						BottangoCore::relayPool->markRelayTeardownReadyToFinalize(bridgeState->respondingPeerID);
					}

					bridgeState->respondingIntent = MessageIntent::Normal;

					// advance broadcast target only after a complete response
					if (broadcastActive())
					{
						bridgeState->broadcastIdx++;
						if (bridgeState->broadcastIdx >= bridgeState->broadcastCount)
						{
							bridgeState->broadcastIdx = 0;
							bridgeState->broadcastCount = 0;
							// broadcast is fully delivered, consume the queue entry now
							BottangoCore::relayPool->outgoingQueue().pop();
						}
					}
				}
				else
				{
					// we just need to keep reading response until we get an EOT from the responding peer
					return;
				}
			}
			else
			{
#ifdef RELAY_COMMS_LOGGING
				Outgoing::printOutputStringFlash(F("ERR: Bridge has no recvBuffer"));
				Outgoing::printLine();
#endif
			}
		}

		OutgoingMessage msg;
		bool gotOutgoing = false;

		// at idle, so check if anything to send out
		if (lineStatus == LineStatus::idle)
		{
			gotOutgoing = tryPrepareNextOutgoing(msg);
			if (gotOutgoing)
			{
				setLineStatus(LineStatus::txOut);
			}
		}

		// need to send
		if (lineStatus == LineStatus::txOut)
		{
			// start transmit
			digitalWrite(RS485_DE_PIN, HIGH); // set line status for tx

			// broadcast uses a snapshot per-target id, unicast uses the message peerId directly
			bridgeState->respondingPeerID = (msg.target == TargetGroup::Unicast) ? msg.peerId : bridgeState->broadcastTargets[bridgeState->broadcastIdx];

			// save the intent of the message as we process it and get response
			bridgeState->respondingIntent = msg.intent;

			// mark the tx on the peer in the pool
			BottangoCore::relayPool->markPeerTx(bridgeState->respondingPeerID);
			if (msg.intent == MessageIntent::Poll)
			{
				BottangoCore::relayPool->markPeerPollOutstanding(bridgeState->respondingPeerID);
			}

			// create a boot request message header
			if (msg.intent == MessageIntent::Boot)
			{
				char macBuffer[20] = {};
				UDIDHelper::convertMACToCStr(BottangoCore::relayPool->getRelay(bridgeState->respondingPeerID)->mac_addr, macBuffer);

				RS485_SERIAL.print(RS485_ENQ);                     // header starts with ENQ char
				RS485_SERIAL.print(macBuffer);                     // then mac
				RS485_SERIAL.print(',');                           // comma for seperation
				RS485_SERIAL.print(bridgeState->respondingPeerID); // then peerID
				RS485_SERIAL.print(RS485_STX);                     // start of text
			}
			// create a command request message header
			else
			{
				RS485_SERIAL.print(RS485_SOH);                     // start of header
				RS485_SERIAL.print(bridgeState->respondingPeerID); // peer id
				RS485_SERIAL.print(RS485_STX);                     // start of text
			}

			// send message payload
#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
			Outgoing::printOutputStringFlash(F("Bridge tx payload: "));
			logBytesWithNL(msg.payload, msg.length);
			Outgoing::printLine();
#endif
			RS485_SERIAL.write(msg.payload, msg.length);

			// send EoT
			RS485_SERIAL.print(RS485_EOT);

			// flush
			RS485_SERIAL.flush();

			// return the line
			digitalWrite(RS485_DE_PIN, LOW); // set line status for rx

			// expect at least one answer, so set line status to rx header in, and reset peer id buffer
			bridgeState->responseWaitStart = millis();
			setLineStatus(LineStatus::rxInHeader);
			peerIdBufferIDX = 0;
			peerIdBuffer[0] = '\0';

			// pop unicast messages immediately, broadcasts pop after the last peer responds
			if (msg.target == TargetGroup::Unicast)
			{
				BottangoCore::relayPool->outgoingQueue().pop();
			}
		}
	}
	// peer
	else
	{
		if (peerState == nullptr)
		{
			return;
		}

		// check peer rx timeout from bridge, while in rx states
		if ((lineStatus == LineStatus::rxInHeader || lineStatus == LineStatus::rxInPayload) &&
			peerState->rxLastByteTime != 0 &&
			(millis() - peerState->rxLastByteTime > RELAY_RS485_RESPONSE_TIMEOUT))
		{
#ifdef RELAY_COMMS_LOGGING
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("ERR: Peer rx timeout, resetting state"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
#endif
			peerState->tossRX = false;
			peerState->txHeaderSent = false;
			peerState->rxLastByteTime = 0;
			peerIdBufferIDX = 0;
			peerIdBuffer[0] = '\0';
			if (recvBuffer)
			{
				recvBuffer->clear();
			}
			setLineStatus(LineStatus::idle);
			return;
		}

		// idle, if we get anything, check the starting char
		// to see if we listen to the header or not
		if (lineStatus == LineStatus::idle)
		{
			while (RS485_SERIAL.available())
			{
				char c = RS485_SERIAL.read();
				peerState->rxLastByteTime = millis();

				// header starting char is peer ack, so a different peer is starting a response, and we can toss it
				if (c == RS485_PEERACK)
				{
					// toss until EOT
					peerState->tossRX = true;
					setLineStatus(LineStatus::rxInPayload);
					break;
				}
				// header starting char is enquiry, start of a registration header
				else if (c == RS485_ENQ)
				{
					// already registered
					if (BottangoCore::hasPeerId)
					{
						// so toss until EOT
						peerState->tossRX = true;
						setLineStatus(LineStatus::rxInPayload);
					}
					// possible registration request, and we're uninit
					else
					{
						// read header (and reset buffer)
						setLineStatus(LineStatus::rxInHeader);
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
					}
					break;
				}
				// start of a real request
				else if (c == RS485_SOH)
				{
					// we have no id yet
					if (!BottangoCore::hasPeerId)
					{
						// so toss until EOT
						peerState->tossRX = true;
						setLineStatus(LineStatus::rxInPayload);
					}
					// can parse the header
					else
					{
						// read header (and reset buffer)
						setLineStatus(LineStatus::rxInHeader);
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
					}
					break;
				}
				else
				{
#ifdef RELAY_COMMS_LOGGING
					Outgoing::toggleOnSecondaryOutgoing();
					Outgoing::printOutputStringFlash(F("WARN: Peer rx unexpected start byte"));
					Outgoing::printLine();
					Outgoing::endToggleOnSecondaryOutgoing();
#endif
					// ignore unexpected byte and keep scanning
				}
			}
		}

		// expect to see more in
		// toss it
		// or process it
		// or if not initialized, check if it's a reg command that matches our mac
		if (lineStatus == LineStatus::rxInHeader)
		{
			// read text
			while (RS485_SERIAL.available())
			{
				char c = RS485_SERIAL.read();
				peerState->rxLastByteTime = millis();

				// reached end of header
				if (c == RS485_STX)
				{
					// we have an id, so this must be a command request header
					// if it were an enq registration header, we'd be tossing
					if (BottangoCore::hasPeerId)
					{
						int peerID = 0;

						// not a valid int?
						if (!parsePeerIdStrict(peerIdBuffer, peerID))
						{
#ifdef RELAY_COMMS_LOGGING
							Outgoing::toggleOnSecondaryOutgoing();
							Outgoing::printOutputStringFlash(F("ERR: Peer rx header invalid peer id"));
							Outgoing::printLine();
							Outgoing::endToggleOnSecondaryOutgoing();
#endif
							peerState->tossRX = true;
						}
						else
						{
							// use payload as a real command if this is our id, otherwise toss
							peerState->tossRX = peerID != BottangoCore::thisPeerID;
						}
						setLineStatus(LineStatus::rxInPayload);
					}
					// we don't have an id, so this is a registration request header
					// if it were a command header, we'd be tossing
					else
					{
						// find the first comma
						char* comma = strchr(peerIdBuffer, ',');
						if (!comma)
						{
							// invalid registration header, toss until EOT
#ifdef RELAY_COMMS_LOGGING
							Outgoing::toggleOnSecondaryOutgoing();
							Outgoing::printOutputStringFlash(F("ERR: Peer rx header invalid (missing comma)"));
							Outgoing::printLine();
							Outgoing::endToggleOnSecondaryOutgoing();
#endif
							peerState->tossRX = true;
							setLineStatus(LineStatus::rxInPayload);
							break;
						}

						*comma = '\0'; // split the c string at the comma

						char* macString = peerIdBuffer; // "AABBCCDDEEFF
						char* idString = comma + 1;     // "42"

						// compare recv mac to my mac
						byte rcvMacAddr[6] = {};
						byte myMacAddr[6] = {};

						// got a bad mac string?
						if (!UDIDHelper::convertCStrToMAC(macString, rcvMacAddr))
						{
#ifdef RELAY_COMMS_LOGGING
							Outgoing::toggleOnSecondaryOutgoing();
							Outgoing::printOutputStringFlash(F("ERR: Peer rx header invalid mac"));
							Outgoing::printLine();
							Outgoing::endToggleOnSecondaryOutgoing();
#endif
							peerState->tossRX = true;
							setLineStatus(LineStatus::rxInPayload);
							break;
						}

						// got mac matches our mac?
						UDIDHelper::getThisDeviceMacAddress(myMacAddr);
						bool macMatch = true;
						for (int i = 0; i < 6; i++)
						{
							if (rcvMacAddr[i] != myMacAddr[i])
							{
								macMatch = false;
							}
						}

						// this is us!
						if (macMatch)
						{
							// ready to parse the payload and register
							int rcvID = 0;

							// got a bad id string?
							if (!parsePeerIdStrict(idString, rcvID))
							{
#ifdef RELAY_COMMS_LOGGING
								Outgoing::toggleOnSecondaryOutgoing();
								Outgoing::printOutputStringFlash(F("ERR: Peer rx header invalid peer id"));
								Outgoing::printLine();
								Outgoing::endToggleOnSecondaryOutgoing();
#endif
								peerState->tossRX = true;
								setLineStatus(LineStatus::rxInPayload);
								break;
							}

							// store this id as our id
							BottangoCore::thisPeerID = rcvID; // store the shorthand id for future headers
							BottangoCore::hasPeerId = true;
							peerState->tossRX = false;
							setLineStatus(LineStatus::rxInPayload); // actually use the payload
						}
						// not us, toss!
						else
						{
							// toss until EOT
							peerState->tossRX = true;
							setLineStatus(LineStatus::rxInPayload);
						}
					}
					break;
				}
				// add peer id chars to header buffer
				else
				{
					// too long header for ID?
					if (peerIdBufferIDX >= (sizeof(peerIdBuffer) - 1))
					{
#ifdef RELAY_COMMS_LOGGING
						Outgoing::toggleOnSecondaryOutgoing();
						Outgoing::printOutputStringFlash(F("ERR: Peer rx header overflow, tossing frame"));
						Outgoing::printLine();
						Outgoing::endToggleOnSecondaryOutgoing();
#endif
						peerState->tossRX = true;
						peerIdBufferIDX = 0;
						peerIdBuffer[0] = '\0';
						setLineStatus(LineStatus::rxInPayload);
						break;
					}

					// store the header
					peerIdBuffer[peerIdBufferIDX] = c;
					peerIdBufferIDX++;
					peerIdBuffer[peerIdBufferIDX] = '\0';
				}
			}
		}

		// got a header we need, or we want to toss until EoT, so read and use payload or toss until EoT
		if (lineStatus == LineStatus::rxInPayload)
		{
			while (RS485_SERIAL.available())
			{
				char c = RS485_SERIAL.read();
				peerState->rxLastByteTime = millis(); //

				// extra logging of what we recv
#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
				if (c != RS485_EOT)
				{
					if (peerState->lineBufferIdx < (sizeof(peerState->lineBuffer) - 1))
					{
						peerState->lineBuffer[peerState->lineBufferIdx] = c;
						peerState->lineBufferIdx++;
						peerState->lineBuffer[peerState->lineBufferIdx] = '\0';
					}

					if (c == '\n')
					{
						Outgoing::toggleOnSecondaryOutgoing();
						if (peerState->tossRX)
						{
							Outgoing::printOutputStringFlash(F("Peer rx payload (tossed): "));
						}
						else
						{
							Outgoing::printOutputStringFlash(F("Peer rx payload: "));
						}
						logStringWithNL(peerState->lineBuffer);
						Outgoing::printLine();
						Outgoing::endToggleOnSecondaryOutgoing();

						peerState->lineBufferIdx = 0;
						peerState->lineBuffer[0] = '\0';
					}
				}
#endif

				// got the EoT for the transmission
				if (c == RS485_EOT)
				{
					// mark response pending if we consumed payload
					peerState->responsePending = !peerState->tossRX;
					if (peerState->responsePending)
					{
						// clear any stale flush, new response should set it explicitly
						peerState->flushToBridge = false;
					}

					// go to idle and wait for a flush signal to send the response (if we were actually spoken to)
					setLineStatus(LineStatus::idle);

					peerState->tossRX = false;
					peerState->txHeaderSent = false;
					peerState->rxLastByteTime = 0;
#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
					peerState->lineBufferIdx = 0;
					peerState->lineBuffer[0] = '\0';
#endif
					break;
				}
				else
				{
					// just throw it away?
					if (peerState->tossRX)
					{
						continue;
					}

					// error state, no space to store it!
					if (recvBuffer && recvBuffer->isFull())
					{
#ifdef RELAY_COMMS_LOGGING
						Outgoing::toggleOnSecondaryOutgoing();
						Outgoing::printOutputStringFlash(F("ERR: Peer rx buffer full, tossing frame"));
						Outgoing::printLine();
						Outgoing::endToggleOnSecondaryOutgoing();
#endif
						peerState->tossRX = true;
						recvBuffer->clear();
						continue;
					}

					// add char to the buffer
					recvBuffer->addChar(c);
					// let bottango core handle pulling from the recv buffer and parsing
				}
			}
		}

		// we're idle, have a response pending, and ready to flush, means we're ready to send everything in the buffer.
		if (lineStatus == LineStatus::idle && peerState->responsePending && peerState->flushToBridge)
		{
			setLineStatus(LineStatus::txOut);
		}

		if (lineStatus == LineStatus::txOut)
		{
			// send the headers
			if (!peerState->txHeaderSent)
			{
				digitalWrite(RS485_DE_PIN, HIGH); // tx on line
				RS485_SERIAL.print(RS485_PEERACK);
				RS485_SERIAL.print(BottangoCore::thisPeerID);
				RS485_SERIAL.print(RS485_STX);
				peerState->txHeaderSent = true;
			}

			char messageBuffer[MAX_COMMAND_LENGTH] = {};

			// print all fully buffered commands
			// leave any partial prints
			while (peerState->txBuffer->getNextTxt(messageBuffer))
			{
#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
				Outgoing::toggleOnSecondaryOutgoing();
				Outgoing::printOutputStringFlash(F("Peer tx payload: "));
				logStringWithNL(messageBuffer);
				Outgoing::printLine();
				Outgoing::endToggleOnSecondaryOutgoing();
#endif
				RS485_SERIAL.print(messageBuffer);
			}

			// close things up
			RS485_SERIAL.print(RS485_EOT);
			RS485_SERIAL.flush();

			digitalWrite(RS485_DE_PIN, LOW); // rx on line

			peerState->flushToBridge = false;
			peerState->responsePending = false;
			setLineStatus(LineStatus::idle);
		}
	}
}

// bridge peer handling
void RelayCommsRS485::registerPeer(const uint8_t* mac_addr)
{
	if (!hasRoleState())
	{
		initializeAsBridge();
	}
}

void RelayCommsRS485::deregisterPeer(const uint8_t* mac_addr)
{

}

// peer comms handling

void RelayCommsRS485::peerPrint(const char* str)
{
	if (peerState != nullptr && peerState->txBuffer && peerState->txBuffer->isFull())
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("ERR: Outgoing peer buffer is full!"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}

	// add to buffer
	if (peerState != nullptr && peerState->txBuffer)
	{
		peerState->txBuffer->addTxt(str);
#if defined(RELAY_COMMS_LOGGING) && defined(RELAY_COMMS_LOGGING_DEBUG)
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("Peer tx queue add: "));
		logStringWithNL(str);
		Outgoing::printLine();
		Outgoing::endToggleOnSecondaryOutgoing();
#endif
	}

	// flush buffer if newline in this text
	bool shouldFlush = false;
	size_t len = strlen(str);
	for (int i = 0; i < len; i++)
	{
		if (str[i] == '\n')
		{
			shouldFlush = true;
			break;
		}
	}
	if (shouldFlush)
	{
		peerFlush();
	}
}

void RelayCommsRS485::peerPrint(const __FlashStringHelper* str)
{
	char command[MAX_COMMAND_LENGTH];
	strncpy_P(command, (PGM_P)str, sizeof(command));
	command[sizeof(command) - 1] = '\0';
	peerPrint(command);
}

void RelayCommsRS485::peerPrintln()
{
	peerPrint("\n");
}

void RelayCommsRS485::peerFlush()
{
	if (peerState != nullptr)
	{
		peerState->flushToBridge = true;
	}
}

bool RelayCommsRS485::peerRecvAvailable()
{
	return !block && recvBuffer && recvBuffer->available();
}

char RelayCommsRS485::peerReadNextChar()
{
	return recvBuffer ? recvBuffer->getNextChar() : '\0';
}

bool RelayCommsRS485::getIsBridge()
{
	return isBridge;
}

#endif
