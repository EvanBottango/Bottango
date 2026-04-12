#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#include "RelayESPNow.h"
#include "RelayChild.h"
#include <stdio.h>
#include "UDIDHelper.h"
#include "Modules/OutgoingSerial.h"
#ifdef ESP32
//#include <esp_system.h>
#endif

#ifdef RELAY_LOGGING
#include "esp_err.h"
#endif

// internal helper to execute bridge to peer transmissions
namespace
{
	enum class EspNowSendResult : uint8_t
	{
		NoPeer,
		QueuedOk,
		QueuedFail
	};

	EspNowSendResult sendToPeer(RelayChildPool* pool, int peerId, const OutgoingMessage& msg)
	{
		uint8_t mac[6] = {};
		bool hasPeer = false;

		// Copy child MAC while locking relay pool so pointer lifetime of the child is safe.
		pool->lockPool();
		RelayChild* child = pool->getPeer(peerId);
		if (child != nullptr)
		{
			memcpy(mac, child->mac_addr, sizeof(mac));
			hasPeer = true;
		}
		pool->unlockPool();

		// couldn't get a peer for the id, error case
		if (!hasPeer)
		{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("WARN: Dropped msg, no relay for id "));
				OutgoingSerial::printOutputStringMem(peerId);
				OutgoingSerial::printLine();
				//Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif // RELAY_LOGGING
			return EspNowSendResult::NoPeer;
		}

		// flag the transmission on the peer
		pool->markPeerTx(peerId);
		if (msg.intent == MessageIntent::Poll)
		{
			// and mark it's a poll
			pool->markPeerPollOutstanding(peerId);
		}

		// transmit
		esp_err_t res = esp_now_send(mac,
			msg.payload,
			msg.length);

		// return status based on transmission success
		if (res == ESP_OK)
		{
			return EspNowSendResult::QueuedOk;
		}
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("ESP-NOW send failed for id "));
			OutgoingSerial::printOutputStringMem(peerId);
			OutgoingSerial::printOutputStringFlash(F(" err "));
			OutgoingSerial::printOutputStringMem(res);
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return EspNowSendResult::QueuedFail;
	}
}

// ----- minimal glue so ESP-NOW C callbacks from initializing espnow can land on "this" -----
static RelayESPNow* gEspNowInstance = nullptr;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
static void EspNow_OnDataSentThunk(const wifi_tx_info_t* tx_info, esp_now_send_status_t status)
{
	if (gEspNowInstance)
	{
		gEspNowInstance->OnDataSent(tx_info, status);
	}
}
static void EspNow_OnDataRecvThunk(const esp_now_recv_info_t* recv_info, const uint8_t* data, int data_len)
{
	if (gEspNowInstance)
	{
		gEspNowInstance->OnDataRecv(recv_info, data, data_len);
	}
}
#else
static void EspNow_OnDataSentThunk(const uint8_t* mac_addr, esp_now_send_status_t status)
{
	if (gEspNowInstance)
	{
		gEspNowInstance->OnDataSent(mac_addr, status);
	}
}
static void EspNow_OnDataRecvThunk(const uint8_t* mac_addr, const uint8_t* data, int data_len)
{
	if (gEspNowInstance)
	{
		gEspNowInstance->OnDataRecv(mac_addr, data, data_len);
	}
}
#endif

// espnow callbacks enque data to be processed in the task, to keep wifi callbacks minimal
namespace
{
	struct EspNowRxPacket
	{
		uint8_t mac[6];
		uint16_t len;
		char payload[MAX_COMMAND_LENGTH];
	};
}

void RelayESPNow::onPhase(Phase p)
{
	if (p != Phase::Communication)
		return;

	if (_relayPool != nullptr)
	{
		_relayPool->update();

		if (isBridge())
		{
			// was stopping all peers, and the queue is now empty
			if (_relayPool->isUninitializing && _relayPool->toPeerQueueEmpty())
			{
				// try stop again, and actually shut down
				// because the queue is empty, it won't abort out
				BottangoCore::request_eStop();
			}
		}
	}
}

// Combined RX/TX task:
// - Drains RX queue so callbacks stay minimal/non-blocking, for both peer and bridge
// - Handles TX only for bridge (peers send via peerFlush)
void RelayESPNow::espNowTxRxTask(void* pvParameters)
{
	// set up bridge vs peer state fields
	RelayESPNow* comms = static_cast<RelayESPNow*>(pvParameters);
	RelayChildPool* pool = comms->_relayPool;
	RelayChildMessageQueue& outQueue = pool->getOutgoingQueue();
	OutgoingMessage msg;
	EspNowRxPacket rxPacket;

	while (true)
	{
		// basic sanity checks
		if (comms == nullptr)
		{
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}

		const bool isBridge = comms->getRole() == RelayRole::Bridge;
		RelayESPNow::BridgeState* bridgeState = comms->_bridgeState;
		RelayESPNow::PeerState* peerState = comms->_peerState;

		// Role state must exist for the active role.
		if (isBridge)
		{
			if (bridgeState == nullptr)
			{
				vTaskDelay(pdMS_TO_TICKS(10));
				continue;
			}
		}
		else
		{
			if (peerState == nullptr)
			{
				vTaskDelay(pdMS_TO_TICKS(10));
				continue;
			}
		}

		// ---- Drain RX (wifi callback only enqueues) ----
		QueueHandle_t rxQueue = isBridge ? bridgeState->rxQueue : peerState->rxQueue;
		volatile bool* rxDropPending = isBridge ? &bridgeState->rxDropPending : &peerState->rxDropPending;
		volatile bool* rxOversizeDropPending = isBridge ? &bridgeState->rxOversizeDropPending : &peerState->rxOversizeDropPending;
		volatile bool* txFailPending = isBridge ? &bridgeState->txFailPending : &peerState->txFailPending;

		if (rxQueue != nullptr)
		{
			// drain a message enqueued from the wifi callback
			while (xQueueReceive(rxQueue, &rxPacket, 0) == pdTRUE)
			{
				// bridge side RX drain
				if (isBridge)
				{
					// Get peer, and hold the pool lock only long enough to resolve and pass up,
					// so pointer lifetime is safe but the lock window is tiny.
					pool->lockPool();
					RelayChild* peer = pool->getPeer(rxPacket.mac);
					if (peer != nullptr)
					{
						peer->passUpCommands(rxPacket.payload);
					}
					pool->unlockPool();

					// log error case if we can't get a peer for the id
#ifdef RELAY_LOGGING
					if (peer == nullptr)
					{
#ifdef TOGGLE_DEBUG
						if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
						{
							//Outgoing::toggleOnSecondaryOutgoing();
							OutgoingSerial::printOutputStringFlash(F("RCV msg "));
							OutgoingSerial::printOutputStringMem(rxPacket.payload);
							OutgoingSerial::printOutputStringFlash(F(" from unknown peer "));
							char cMAC[20];
							UDIDHelper::convertMACToCStr(rxPacket.mac, cMAC);
							OutgoingSerial::printOutputStringMem(cMAC);
							OutgoingSerial::printLine();
							//Outgoing::endToggleOnSecondaryOutgoing();
						}
					}
#endif // RELAY_LOGGING
				}
				// peer side rx drain
				else
				{
					// Peer RX: buffer for main loop consumption.

					// rx queue full error state
					if (peerState->rxBuffer && peerState->rxBuffer->isFull())
					{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
						if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
						{
							//Outgoing::toggleOnSecondaryOutgoing();
							OutgoingSerial::printOutputStringFlash(F("ERR: Msg recieved, in buffer full: "));
							OutgoingSerial::printOutputStringMem(rxPacket.payload);
							OutgoingSerial::printLine();
							//Outgoing::endToggleOnSecondaryOutgoing();
						}
#endif // RELAY_LOGGING
						continue;
					}
					else
					{
						// add the rx text to rxBuffer to be processed as called by Bottango Core
						if (peerState->rxBuffer)
						{
							peerState->rxBuffer->addTxt(rxPacket.payload);
						}
					}
				}
			}
		}

		// ---- Log RX drops as error state if flag was set ----

		// callback got too long text
		if (rxOversizeDropPending != nullptr && *rxOversizeDropPending)
		{
			*rxOversizeDropPending = false;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("ERR: ESP-NOW RX dropped (oversize)"));
				OutgoingSerial::printLine();
				//Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif // RELAY_LOGGING
		}

		// callback couldn't enque, the callback text queue was full
		if (rxDropPending != nullptr && *rxDropPending)
		{
			*rxDropPending = false;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("ERR: ESP-NOW RX dropped"));
				OutgoingSerial::printLine();
				//Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif // RELAY_LOGGING
		}

		// TX failed
		if (txFailPending != nullptr && *txFailPending)
		{
			*txFailPending = false;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
			{
				//Outgoing::toggleOnSecondaryOutgoing();
				OutgoingSerial::printOutputStringFlash(F("ERR: ESP-NOW TX failed"));
				OutgoingSerial::printLine();
				//Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif // RELAY_LOGGING
		}

		// ---- Process TX (bridge only) ----
		// peer handles tx in peer flush
		if (isBridge && outQueue.peek(msg))
		{
			// send to one target?
			if (msg.target == TargetGroup::Unicast)
			{
				bridgeState->broadcastActive = false;

				EspNowSendResult res = sendToPeer(pool, msg.peerId, msg);

				// pop the message unless we got a radio failure
				if (res == EspNowSendResult::NoPeer || res == EspNowSendResult::QueuedOk)
				{
					outQueue.pop();
				}
			}
			// broadcast to multiple targets
			else
			{
				// no broadcast already active? set one up!
				if (!bridgeState->broadcastActive)
				{
					bridgeState->broadcastActive = true;
					bridgeState->broadcastNextIndex = 0;
					bridgeState->broadcastTargetCount = 0;

					// broadcast to all connected or unconnected peers?
					if (msg.target == TargetGroup::BroadcastUnconnected)
					{
						pool->getUnconnectedPeerIds(bridgeState->broadcastTargetIds, bridgeState->broadcastTargetCount);
					}
					else
					{
						pool->getConnectedPeerIds(bridgeState->broadcastTargetIds, bridgeState->broadcastTargetCount);
					}
				}

				// no peers of the requred broadcast target type? just pop and move on
				if (bridgeState->broadcastTargetCount == 0)
				{
					bridgeState->broadcastActive = false;
					outQueue.pop();
				}
				else
				{
					// iterate through the peers of the broadcast target type, and tx to each
					while (bridgeState->broadcastNextIndex < bridgeState->broadcastTargetCount)
					{
						int peerId = bridgeState->broadcastTargetIds[bridgeState->broadcastNextIndex];
						EspNowSendResult res = sendToPeer(pool, peerId, msg);

						// radio fail? Let's come back and try again next tick
						if (res == EspNowSendResult::QueuedFail)
						{
							break;
						}
						bridgeState->broadcastNextIndex++;
					}

					// pop once we've broadcast to all peers that need the message
					if (bridgeState->broadcastNextIndex >= bridgeState->broadcastTargetCount)
					{
						bridgeState->broadcastActive = false;
						outQueue.pop();
					}
				}
			}
		}

		// ---- Idle ----
		// Minimum delay between iterations to keep TX spacing consistent.
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

// Callback after data is sent
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
// Arduino-ESP32 3.x (IDF 5.x): first param is wifi_tx_info_t*
void RelayESPNow::OnDataSent(const wifi_tx_info_t* tx_info, esp_now_send_status_t status)
{
#else
void RelayESPNow::OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
#endif // ESP_ARDUINO_VERSION >= 3
#endif // ESP_ARDUINO_VERSION_MAJOR
	if (status == ESP_NOW_SEND_SUCCESS)
	{
		return;
	}

	if (_bridgeState != nullptr)
	{
		_bridgeState->txFailPending = true;
	}
	else if (_peerState != nullptr)
	{
		_peerState->txFailPending = true;
	}
}

// Callback when data is received
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void RelayESPNow::OnDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int data_len)
{
	// Access the MAC address from recv_info->src_addr
	const uint8_t* mac_addr = recv_info->src_addr;
#else
void RelayESPNow::OnDataRecv(const uint8_t * mac_addr, const uint8_t * data, int data_len)
{
#endif // ESP_ARDUINO_VERSION >= 3
#endif // ESP_ARDUINO_VERSION_MAJOR

	// Keep callback lightweight: copy into RX queue and return.
	QueueHandle_t rxQueue = nullptr;
	volatile bool* rxDropPending = nullptr;
	volatile bool* rxOversizeDropPending = nullptr;

	if (_relayRole == RelayRole::Bridge && _bridgeState != nullptr)
	{
		rxQueue = _bridgeState->rxQueue;
		rxDropPending = &_bridgeState->rxDropPending;
		rxOversizeDropPending = &_bridgeState->rxOversizeDropPending;
	}
	else if (_relayRole == RelayRole::Peer && _peerState != nullptr)
	{
		rxQueue = _peerState->rxQueue;
		rxDropPending = &_peerState->rxDropPending;
		rxOversizeDropPending = &_peerState->rxOversizeDropPending;
	}

	if (rxQueue == nullptr)
	{
		if (rxDropPending != nullptr)
		{
			*rxDropPending = true;
		}
		return;
	}

	// Copy payload into a fixed packet struct to process in the rxtx task.
	EspNowRxPacket packet = {};
	memcpy(packet.mac, mac_addr, sizeof(packet.mac));

	if (data_len >= MAX_COMMAND_LENGTH)
	{
		if (rxOversizeDropPending != nullptr)
		{
			*rxOversizeDropPending = true;
		}
		return;
	}

	packet.len = data_len;
	memcpy(packet.payload, data, data_len);
	packet.payload[data_len] = '\0';

	if (xQueueSend(rxQueue, &packet, 0) != pdTRUE)
	{
		if (rxDropPending != nullptr)
		{
			*rxDropPending = true;
		}
	}
}

bool RelayESPNow::hasRoleState()
{
	return _bridgeState != nullptr || _peerState != nullptr;
}

void RelayESPNow::initializeAsBridge()
{
	if (hasRoleState())
	{
		return;
	}

	_bridgeState = new BridgeState();
	_bridgeState->rxQueue = xQueueCreate(ESPNOW_RX_QUEUE_DEPTH, sizeof(EspNowRxPacket));
	configASSERT(_bridgeState->rxQueue);

	_relayRole = RelayRole::Bridge;
	initConnection();

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
	{
		//Outgoing::toggleOnSecondaryOutgoing();
		OutgoingSerial::printOutputStringFlash(F("Bridge init complete"));
		OutgoingSerial::printLine();
		//Outgoing::endToggleOnSecondaryOutgoing();
	}
#endif // RELAY_LOGGING

	xTaskCreate(
		espNowTxRxTask,
		"espNowTxRx",
		2048,
		this, // pass comms for rx/tx task
		1,
		nullptr);
}

void RelayESPNow::initializeAsPeer()
{
	if (hasRoleState())
	{
		return;
	}

	_peerState = new PeerState();
	_peerState->rxQueue = xQueueCreate(ESPNOW_RX_QUEUE_DEPTH, sizeof(EspNowRxPacket));
	configASSERT(_peerState->rxQueue);

	_peerState->rxBuffer = new TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>();
	_peerState->txBuffer = new TxtBuffer<TXT_BUFFER_SIZE_TX_COMMS>();

	_relayRole = RelayRole::Peer;
	initConnection();

	uint8_t bridgeMAC[6];
	bool gotBridgeMac = PersistentConfigUtil::getBridgeMacAddress(bridgeMAC);

	// Fatal: no parent to talk to. Restart to surface the error and retry init.
	if (!gotBridgeMac)
	{
		//Outgoing::toggleOnSecondaryOutgoing();
		OutgoingSerial::printOutputStringFlash(F("ERR: No bridge mac for peer"));
		OutgoingSerial::printLine();
		OutgoingSerial::flush();
		//Outgoing::endToggleOnSecondaryOutgoing();
#ifdef ESP32
		delay(100);
		esp_restart();
#endif // ESP32
		return;
	}

	memcpy(_peerInfo.peer_addr, bridgeMAC, 6);
	_peerInfo.channel = ESPNOW_CHANNEL;
	_peerInfo.encrypt = false;

	// Add bridge ("parent peer")
	if (esp_now_add_peer(&_peerInfo) != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Failed to add parent peer"));
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return;
	}

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
	{
		char macC[20];
		UDIDHelper::convertMACToCStr(bridgeMAC, macC);
		//OutgoingSerial::toggleOnSecondaryOutgoing();
		OutgoingSerial::printOutputStringFlash(F("Peer callback init from bridge with MAC "));
		OutgoingSerial::printOutputStringMem(macC);
		OutgoingSerial::printLine();
		//Outgoing::endToggleOnSecondaryOutgoing();
	}

#endif // RELAY_LOGGING

	xTaskCreate(
		espNowTxRxTask,
		"espNowTxRx",
		2048,
		this, // pass comms for rx/tx task
		1,
		nullptr);
}

void RelayESPNow::initConnection()
{
	gEspNowInstance = this;

	if (_wifiInitialized)
	{
		return;
	}

	// make sure netif + event loop exist
	if (!_netifReady)
	{
		esp_err_t ret;

		ret = esp_netif_init();
		if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
		{
			return;
		}

		ret = esp_event_loop_create_default();
		if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
		{
			return;
		}

		// Create default STA netif if not already present
		if (!esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"))
		{
			esp_netif_create_default_wifi_sta();
		}
		_netifReady = true;
	}

	// Set device as a Wi-Fi Station
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_err_t ret = esp_wifi_init(&cfg);
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Error initializing WiFi: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return;
	}

	ret = esp_wifi_set_mode(WIFI_MODE_STA);
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Error setting WiFi mode: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return;
	}

	// Start the Wi-Fi driver so the interface becomes fully active
	ret = esp_wifi_start();
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Error starting WiFi: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return;
	}

	// todo, test is this needed, does it mess things up?
	delay(200);

	// Now initialize ESP-NOW
	if (esp_now_init() != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Error initializing ESP-NOW: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif // RELAY_LOGGING
		return;
	}

	esp_now_register_send_cb(EspNow_OnDataSentThunk); // register send callback
	esp_now_register_recv_cb(EspNow_OnDataRecvThunk); // register recv callback

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
	{
		//Outgoing::toggleOnSecondaryOutgoing();
		OutgoingSerial::printOutputStringFlash(F("ESP-NOW radio initialized"));
		OutgoingSerial::printLine();
		//Outgoing::endToggleOnSecondaryOutgoing();
	}
#endif // RELAY_LOGGING

	_wifiInitialized = true;
}

// peer management, as bridge
void RelayESPNow::registerPeer(const uint8_t * mac_addr)
{
	if (!hasRoleState())
	{
		initializeAsBridge();
	}

	memset(&_peerInfo, 0, sizeof(esp_now_peer_info_t)); // Zero out the structure

	// Register peer
	memcpy(_peerInfo.peer_addr, mac_addr, 6);
	_peerInfo.channel = ESPNOW_CHANNEL;
	_peerInfo.encrypt = false;

	// Add peer
	esp_err_t ret = esp_now_add_peer(&_peerInfo);

	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			OutgoingSerial::printOutputStringFlash(F("Failed to add peer: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
		}
#endif // RELAY_LOGGING
		return;
	}

	// peer added success
}

void RelayESPNow::deregisterPeer(const uint8_t * mac_addr)
{
	// Deregister peer

	// del peer
	esp_err_t ret = esp_now_del_peer(mac_addr);
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			//Outgoing::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Failed to remove peer: "));
			OutgoingSerial::printOutputStringMem(esp_err_to_name(ret));
			OutgoingSerial::printOutputStringFlash(F(" "));
			OutgoingSerial::printOutputStringMem(String(ret, HEX).c_str());
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}
}

// comms, as peer
void RelayESPNow::peerPrint(const char* str)
{
	if (_peerState != nullptr && _peerState->txBuffer && _peerState->txBuffer->isFull())
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
		{
			//OutgoingRelay::toggleOnSecondaryOutgoing();
			OutgoingSerial::printOutputStringFlash(F("Outgoing peer buffer is full!"));
			OutgoingSerial::printLine();
			//Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif // RELAY_LOGGING
		return;
	}

	// add to buffer
	if (_peerState != nullptr && _peerState->txBuffer)
	{
		_peerState->txBuffer->addTxt(str);
	}

	// flush whole buffer to ESP-Now if newline in this text
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

void RelayESPNow::peerPrint(const __FlashStringHelper * str)
{
	char command[MAX_COMMAND_LENGTH];
	strncpy_P(command, (PGM_P)str, sizeof(command));
	command[sizeof(command) - 1] = '\0';
	peerPrint(command);
}

void RelayESPNow::peerPrintln()
{
	peerPrint("\n");
}

void RelayESPNow::peerFlush()
{
	while (_peerState != nullptr && _peerState->txBuffer && _peerState->txBuffer->available())
	{
		char message[MAX_COMMAND_LENGTH];
		_peerState->txBuffer->getNextTxt(message);

		if (strlen(message) == 0)
		{
			// todo error here
			return;
		}

		// Outgoing::setSecondaryPeerOutgoing(true);
		// Outgoing::printOutputStringFlash(F("peer pass up message: "));
		// Outgoing::printOutputStringMem(message);
		// Outgoing::printLine();
		// Outgoing::setSecondaryPeerOutgoing(false);

		// Send message via ESP-NOW
		esp_err_t result = esp_now_send(_peerInfo.peer_addr, (uint8_t*)message, strlen(message));

#ifdef RELAY_LOGGING
		if (result != ESP_OK)
		{
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif // TOGGLE_DEBUG
			{
				//Outgoing::setSecondaryPeerOutgoing(true);
				OutgoingSerial::printOutputStringFlash(F("Sent with Failure: "));
				OutgoingSerial::printOutputStringMem(message);
				OutgoingSerial::printLine();
				//Outgoing::setSecondaryPeerOutgoing(false);
			}
		}
#endif // RELAY_LOGGING
	}
}

bool RelayESPNow::peerRecvAvailable()
{
	return _peerState != nullptr && _peerState->rxBuffer && _peerState->rxBuffer->available();
}

char RelayESPNow::peerReadNextChar()
{
	return (_peerState != nullptr && _peerState->rxBuffer) ? _peerState->rxBuffer->getNextChar() : '\0';
}
#endif