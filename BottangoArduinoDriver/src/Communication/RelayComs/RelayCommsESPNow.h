#pragma once

#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "IRelayComms.h"
#include <freertos/queue.h>
#include "../../Util/TxtBuffer.h"
#include "../../System/BottangoCore.h"


class RelayCommsESPNow : public IRelayComms
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

	bool getIsBridge() override;
	void update() override;

	// ---- ESP-NOW specific ----
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
	void OnDataSent(const wifi_tx_info_t* tx_info, esp_now_send_status_t status);
	void OnDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int data_len);
#else
	void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
	void OnDataRecv(const uint8_t* mac_addr, const uint8_t* data, int data_len);
#endif

private:
	// initialization and basic lifecycle
	bool wifiInitialized = false;
	bool isBridge = false;
	esp_now_peer_info_t peerInfo = {};
	bool netifReady = false;

	void initConnection();
	static void espNowTxRxTask(void* pvParameters);

	// data state for bridge branch
	struct BridgeState
	{
		// enque from radio callback to process in task
		QueueHandle_t rxQueue = nullptr;

		// error flags from radio callbacks
		volatile bool rxDropPending = false;
		volatile bool rxOversizeDropPending = false;
		volatile bool txFailPending = false;

		// tracking of muti-cast state
		int broadcastTargetIds[MAX_RELAY_CHILD] = {};
		uint8_t broadcastTargetCount = 0;
		uint8_t broadcastNextIndex = 0;
		bool broadcastActive = false;
	};

	// data state for peer branch
	struct PeerState
	{
		// enque from radio callback to process in task
		QueueHandle_t rxQueue = nullptr;

		// error flags from radio callbacks
		volatile bool rxDropPending = false;
		volatile bool rxOversizeDropPending = false;
		volatile bool txFailPending = false;

		// enque from radio callback to process in task
		TxtBuffer<TXT_BUFFER_SIZE_RX_COMMS>* rxBuffer = nullptr;
		TxtBuffer<TXT_BUFFER_SIZE_TX_COMMS>* txBuffer = nullptr;
	};

	// roll state
	BridgeState* bridgeState = nullptr;
	PeerState* peerState = nullptr;
	bool hasRoleState();
};

#endif // RELAY_COMS_ESPNOW
