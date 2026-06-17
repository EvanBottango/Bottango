#include "RelayCommsESPNow.h"
#if defined(RELAY_COMS_ESPNOW)

#include <stdio.h>
#ifdef ESP32
#include <esp_system.h>
#endif

#include "RelayChild.h"
#include "../../Util/UDIDHelper.h"

#ifdef RELAY_LOGGING
#include "esp_err.h"
#endif

// internal helper to execute bridge to peer transmissions
namespace
{
	constexpr uint32_t ESPNOW_TX_RX_TASK_STACK_BYTES = 3100;

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
		bool skippedTeardownPeer = false;

		// Copy child MAC while locking relay pool so pointer lifetime of the child is safe.
		pool->lockPool();
		RelayChild* child = pool->getRelay(peerId);
		if (child != nullptr)
		{
			// dont send messages to peers that are tearing down
			if (child->teardown && msg.intent != MessageIntent::Teardown)
			{
				skippedTeardownPeer = true;
			}
			else
			{
				memcpy(mac, child->mac_addr, sizeof(mac));
				hasPeer = true;
			}
		}
		pool->unlockPool();

		// couldn't get a peer for the id, error case except for peer teardown
		if (!hasPeer)
		{
			// ok to have skipped if this peer is tearing down
			if (skippedTeardownPeer)
			{
				return EspNowSendResult::NoPeer;
			}

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			// log the error
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::toggleOnSecondaryOutgoing();
				Outgoing::printOutputStringFlash(F("WARN: Dropped msg, no relay for id "));
				Outgoing::printOutputStringMem(peerId);
				Outgoing::printLine();
				Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif
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
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("ESP-NOW send failed for id "));
			Outgoing::printOutputStringMem(peerId);
			Outgoing::printOutputStringFlash(F(" err "));
			Outgoing::printOutputStringMem(res);
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif
		return EspNowSendResult::QueuedFail;
	}
}

// ----- minimal glue so ESP-NOW C callbacks from initializing espnow can land on "this" -----
static RelayCommsESPNow* gEspNowInstance = nullptr;

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

// Combined RX/TX task:
// - Drains RX queue so callbacks stay minimal/non-blocking, for both peer and bridge
// - Handles TX only for bridge (peers send via peerFlush)
void RelayCommsESPNow::espNowTxRxTask(void* pvParameters)
{
	// set up bridge vs peer state fields
	RelayCommsESPNow* comms = static_cast<RelayCommsESPNow*>(pvParameters);
	RelayChildPool* pool = BottangoCore::relayPool;
	RelayChildMessageQueue& queue = pool->outgoingQueue();
	OutgoingMessage msg;
	EspNowRxPacket rxPacket;

	for (;;)
	{
		// basic sanity checks

		if (comms == nullptr)
		{
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}

		const bool isBridge = comms->isBridge;
		RelayCommsESPNow::BridgeState* bridgeState = comms->bridgeState;
		RelayCommsESPNow::PeerState* peerState = comms->peerState;

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
					RelayChild* relay = pool->getRelay(rxPacket.mac);
					if (relay != nullptr)
					{
						relay->passUpCommands(rxPacket.payload);
					}
					pool->unlockPool();

					// log error case if we can't get a peer for the id
#ifdef RELAY_LOGGING
					if (relay == nullptr)
					{
#ifdef TOGGLE_DEBUG
						if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
						{
							Outgoing::toggleOnSecondaryOutgoing();
							Outgoing::printOutputStringFlash(F("RCV msg "));
							Outgoing::printOutputStringMem(rxPacket.payload);
							Outgoing::printOutputStringFlash(F(" from unknown peer "));
							char cMAC[20];
							UDIDHelper::convertMACToCStr(rxPacket.mac, cMAC);
							Outgoing::printOutputStringMem(cMAC);
							Outgoing::printLine();
							Outgoing::endToggleOnSecondaryOutgoing();
						}
					}
#endif
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
#endif
						{
							Outgoing::toggleOnSecondaryOutgoing();
							Outgoing::printOutputStringFlash(F("ERR: Msg recieved, in buffer full: "));
							Outgoing::printOutputStringMem(rxPacket.payload);
							Outgoing::printLine();
							Outgoing::endToggleOnSecondaryOutgoing();
						}
#endif
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
#endif
			{
				Outgoing::toggleOnSecondaryOutgoing();
				Outgoing::printOutputStringFlash(F("ERR: ESP-NOW RX dropped (oversize)"));
				Outgoing::printLine();
				Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif
		}

		// callback couldn't enque, the callback text queue was full
		if (rxDropPending != nullptr && *rxDropPending)
		{
			*rxDropPending = false;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::toggleOnSecondaryOutgoing();
				Outgoing::printOutputStringFlash(F("ERR: ESP-NOW RX dropped"));
				Outgoing::printLine();
				Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif
		}

		// TX failed
		if (txFailPending != nullptr && *txFailPending)
		{
			*txFailPending = false;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::toggleOnSecondaryOutgoing();
				Outgoing::printOutputStringFlash(F("ERR: ESP-NOW TX failed"));
				Outgoing::printLine();
				Outgoing::endToggleOnSecondaryOutgoing();
			}
#endif
		}

		// ---- Process TX (bridge only) ----
		// peer handles tx in peer flush
		if (isBridge && queue.peek(msg))
		{
			// send to one target?
			if (msg.target == TargetGroup::Unicast)
			{
				bridgeState->broadcastActive = false;

				EspNowSendResult res = sendToPeer(pool, msg.peerId, msg);

				// pop the message unless we got a radio failure
				if (res == EspNowSendResult::NoPeer || res == EspNowSendResult::QueuedOk)
				{
					queue.pop();

					// let the pool know the teardown message was sent, and we can finalize tearing it down
					if (msg.intent == MessageIntent::Teardown)
					{
						pool->markRelayTeardownReadyToFinalize(msg.peerId);
					}
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
						pool->getUnconnectedRelayIds(bridgeState->broadcastTargetIds, bridgeState->broadcastTargetCount);
					}
					else
					{
						bool includeTeardown = msg.intent == MessageIntent::Teardown;
						pool->getConnectedRelayIds(bridgeState->broadcastTargetIds, bridgeState->broadcastTargetCount, includeTeardown);
					}
				}

				// no peers of the requred broadcast target type? just pop and move on
				if (bridgeState->broadcastTargetCount == 0)
				{
					bridgeState->broadcastActive = false;
					queue.pop();
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

						if (msg.intent == MessageIntent::Teardown)
						{
							pool->markRelayTeardownReadyToFinalize(peerId);
						}

						bridgeState->broadcastNextIndex++;
					}

					// pop once we've broadcast to all peers that need the message
					if (bridgeState->broadcastNextIndex >= bridgeState->broadcastTargetCount)
					{
						bridgeState->broadcastActive = false;
						queue.pop();
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
void RelayCommsESPNow::OnDataSent(const wifi_tx_info_t* tx_info, esp_now_send_status_t status)
#else
// Arduino-ESP32 2.x and earlier
void RelayCommsESPNow::OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
#endif
#else
// Fallback for environments without version macros
void RelayCommsESPNow::OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
#endif
{
	if (status == ESP_NOW_SEND_SUCCESS)
	{
		return;
	}

	if (bridgeState != nullptr)
	{
		bridgeState->txFailPending = true;
	}
	else if (peerState != nullptr)
	{
		peerState->txFailPending = true;
	}
}

// Callback when data is received
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void RelayCommsESPNow::OnDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int data_len)
{
	// Access the MAC address from recv_info->src_addr
	const uint8_t* mac_addr = recv_info->src_addr;
#else
void RelayCommsESPNow::OnDataRecv(const uint8_t * mac_addr, const uint8_t * data, int data_len)
{
#endif
#endif
	// Keep callback lightweight: copy into RX queue and return.
	QueueHandle_t rxQueue = nullptr;
	volatile bool* rxDropPending = nullptr;
	volatile bool* rxOversizeDropPending = nullptr;

	if (isBridge && bridgeState != nullptr)
	{
		rxQueue = bridgeState->rxQueue;
		rxDropPending = &bridgeState->rxDropPending;
		rxOversizeDropPending = &bridgeState->rxOversizeDropPending;
	}
	else if (!isBridge && peerState != nullptr)
	{
		rxQueue = peerState->rxQueue;
		rxDropPending = &peerState->rxDropPending;
		rxOversizeDropPending = &peerState->rxOversizeDropPending;
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

bool RelayCommsESPNow::hasRoleState()
{
	return bridgeState != nullptr || peerState != nullptr;
}

void RelayCommsESPNow::initializeAsBridge()
{
	if (hasRoleState())
	{
		return;
	}
	if (bridgeState == nullptr)
	{
		bridgeState = new BridgeState();
	}
	if (bridgeState->rxQueue == nullptr)
	{
		bridgeState->rxQueue = xQueueCreate(ESPNOW_RX_QUEUE_DEPTH, sizeof(EspNowRxPacket));
		configASSERT(bridgeState->rxQueue);
	}
	isBridge = true;
	initConnection();

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("Bridge init complete"));
		Outgoing::printLine();
		Outgoing::endToggleOnSecondaryOutgoing();
	}

#endif

	xTaskCreate(
		espNowTxRxTask,
		"espNowTxRx",
		ESPNOW_TX_RX_TASK_STACK_BYTES,
		this, // pass comms for rx/tx task
		1,
		nullptr);
}

void RelayCommsESPNow::initializeAsPeer()
{
	if (hasRoleState())
	{
		return;
	}
	if (peerState == nullptr)
	{
		peerState = new PeerState();
	}
	if (peerState->rxQueue == nullptr)
	{
		peerState->rxQueue = xQueueCreate(ESPNOW_RX_QUEUE_DEPTH, sizeof(EspNowRxPacket));
		configASSERT(peerState->rxQueue);
	}

	peerState->rxBuffer = new TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>();
	peerState->txBuffer = new TxtBuffer<TXT_BUFFER_SIZE_TX_COMMS>();
	isBridge = false;

	initConnection();

	uint8_t mac[6];
	bool gotBridgeMac = PersistentConfigUtil::getBridgeMacAddress(mac);

	// Fatal: no parent to talk to. Restart to surface the error and retry init.
	if (!gotBridgeMac)
	{
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("ERR: No bridge mac for peer"));
		Outgoing::printLine();
		Outgoing::flush();
		Outgoing::endToggleOnSecondaryOutgoing();
#ifdef ESP32
		delay(100);
		esp_restart();
#endif
		return;
	}

	memcpy(peerInfo.peer_addr, mac, 6);
	peerInfo.channel = ESPNOW_CHANNEL;
	peerInfo.encrypt = false;

	// Add parent peer
	if (esp_now_add_peer(&peerInfo) != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Failed to add parent peer"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}
#endif
		return;
	}

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		char macC[20];
		UDIDHelper::convertMACToCStr(mac, macC);
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("Peer callback init from bridge with MAC "));
		Outgoing::printOutputStringMem(macC);
		Outgoing::printLine();
		Outgoing::endToggleOnSecondaryOutgoing();
	}

#endif

	xTaskCreate(
		espNowTxRxTask,
		"espNowTxRx",
		ESPNOW_TX_RX_TASK_STACK_BYTES,
		this, // pass comms for rx/tx task
		1,
		nullptr);
}

void RelayCommsESPNow::initConnection()
{
	gEspNowInstance = this;

	if (wifiInitialized)
	{
		return;
	}

	// make sure netif + event loop exist
	if (!netifReady)
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
		netifReady = true;
	}

	// Set device as a Wi-Fi Station
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_err_t ret = esp_wifi_init(&cfg);
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Error initializing WiFi: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}

	ret = esp_wifi_set_mode(WIFI_MODE_STA);
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Error setting WiFi mode: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}

	// Start the Wi-Fi driver so the interface becomes fully active
	ret = esp_wifi_start();
	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Error starting WiFi: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
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
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Error initializing ESP-NOW: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}

	esp_now_register_send_cb(EspNow_OnDataSentThunk); // register send callback
	esp_now_register_recv_cb(EspNow_OnDataRecvThunk); // register recv callback

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::toggleOnSecondaryOutgoing();
		Outgoing::printOutputStringFlash(F("ESP-NOW radio initialized"));
		Outgoing::printLine();
		Outgoing::endToggleOnSecondaryOutgoing();
	}
#endif

	wifiInitialized = true;
}

// peer management, as bridge

void RelayCommsESPNow::registerPeer(const uint8_t * mac_addr)
{
	if (!hasRoleState())
	{
		initializeAsBridge();
	}

	memset(&peerInfo, 0, sizeof(esp_now_peer_info_t)); // Zero out the structure

	// Register peer
	memcpy(peerInfo.peer_addr, mac_addr, 6);
	peerInfo.channel = ESPNOW_CHANNEL;
	peerInfo.encrypt = false;

	// Add peer
	esp_err_t ret = esp_now_add_peer(&peerInfo);

	if (ret != ESP_OK)
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Failed to add peer: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}

	// peer added success
}

void RelayCommsESPNow::deregisterPeer(const uint8_t * mac_addr)
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
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Failed to remove peer: "));
			Outgoing::printOutputStringMem(esp_err_to_name(ret));
			Outgoing::printOutputStringFlash(F(" "));
			Outgoing::printOutputStringMem(String(ret, HEX).c_str());
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
		}

#endif
		return;
	}
}

// comms, as peer

void RelayCommsESPNow::peerPrint(const char* str)
{
	if (peerState != nullptr && peerState->txBuffer && peerState->txBuffer->isFull())
	{
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("Outgoing peer buffer is full!"));
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
	}

	// flush whole buffer to espnow if newline in this text
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

void RelayCommsESPNow::peerPrint(const __FlashStringHelper * str)
{
	char command[MAX_COMMAND_LENGTH];
	strncpy_P(command, (PGM_P)str, sizeof(command));
	command[sizeof(command) - 1] = '\0';
	peerPrint(command);
}

void RelayCommsESPNow::peerPrintln()
{
	peerPrint("\n");
}

bool RelayCommsESPNow::getIsBridge()
{
	return isBridge;
}

void RelayCommsESPNow::peerFlush()
{
	while (peerState != nullptr && peerState->txBuffer && peerState->txBuffer->available())
	{
		char message[MAX_COMMAND_LENGTH];
		peerState->txBuffer->getNextTxt(message);

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
		esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t*)message, strlen(message));

		if (result != ESP_OK)
		{

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
			{
				Outgoing::setSecondaryPeerOutgoing(true);
				Outgoing::printOutputStringFlash(F("Sent with Failure: "));
				Outgoing::printOutputStringMem(message);
				Outgoing::printLine();
				Outgoing::setSecondaryPeerOutgoing(false);
			}

#endif
		}
	}
}

bool RelayCommsESPNow::peerRecvAvailable()
{
	return peerState != nullptr && peerState->rxBuffer && peerState->rxBuffer->available();
}

char RelayCommsESPNow::peerReadNextChar()
{
	return (peerState != nullptr && peerState->rxBuffer) ? peerState->rxBuffer->getNextChar() : '\0';
}

void RelayCommsESPNow::update()
{
	// no op
}
#endif
