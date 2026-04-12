#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#pragma once

#include <Arduino.h>
#include "Relay.h"

#include <esp_now.h>
#include "esp_wifi.h"
#include "../../DataSource/TxtBuffer.h"
#include "BottangoCore.h"
#include <freertos/queue.h>

// ToDo:
// Mir gefällt nicht, dass diese Klasse hier Bridge UND Peer darstellt. Das sollte getrennt werden.
// Auch die Relay Grundklasse enthält schon beides.
// Der Overhead ist zwar minimal, aber es ist konzeptionell sauberer, wenn es zwei Klassen gibt, die von Relay erben: RelayCommsESPNowBridge und RelayCommsESPNowPeer
class RelayESPNow : public Relay
{
public:
	void onPhase(Phase p) override;

	// ==== Bridge ====
	void initializeAsBridge() override;
	void registerPeer(const uint8_t* udid) override;
	void deregisterPeer(const uint8_t* udid) override;

	// ==== Peer ====
	void initializeAsPeer() override;
	void peerPrint(const char* str) override;
	void peerPrint(const __FlashStringHelper* str) override;
	void peerPrintln() override;
	void peerFlush() override;
	bool peerRecvAvailable() override;
	char peerReadNextChar() override;

	// ==== ESP-NOW specific ====
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
	void OnDataSent(const wifi_tx_info_t* tx_info, esp_now_send_status_t status);
	void OnDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int data_len);
#else
	void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
	void OnDataRecv(const uint8_t* mac_addr, const uint8_t* data, int data_len);
#endif // ESP_ARDUINO_VERSION_MAJOR

private:
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
	BridgeState* _bridgeState = nullptr;
	PeerState* _peerState = nullptr;

	// initialization and basic lifecycle
	bool _wifiInitialized = false;

	//bool isBridge = false;
	esp_now_peer_info_t _peerInfo = {};
	bool _netifReady = false;

	void initConnection();
	static void espNowTxRxTask(void* pvParameters);	
	bool hasRoleState();
};

#endif // RELAY_COMS_ESPNOW