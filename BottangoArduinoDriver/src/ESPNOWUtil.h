#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#ifndef ESPNowUtil_h
#define ESPNowUtil_h

#include "Arduino.h"
#include <esp_now.h>
#include "esp_wifi.h"
#include "BottangoCore.h"
#include "TxtBuffer.h"

namespace ESPNowUtil
{
    // Bridge
    void initializeESPNowAsBridge();
    void registerPeer(const uint8_t *mac_addr);
    void deregisterPeer(const uint8_t *mac_addr);

    // Peer
    void initializeESPNowAsPeer();
    void peerPrint(const char *str);
    void peerPrint(const __FlashStringHelper *str);
    void peerPrintln();
    void peerFlush();
    bool peerRecvAvailable();
    char peerReadNextChar();

    void initESPNowConnection();
    void getThisDeviceMACAddress(uint8_t mac[6]);
    void convertCStrToMac(const char *str, uint8_t mac[6]);
    void convertMacToCStr(const uint8_t mac[6], char *str);

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // Arduino-ESP32 3.x (IDF 5.x): first param is wifi_tx_info_t*
    void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status);
#else
    // Arduino-ESP32 2.x and earlier
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
#else
    // Fallback for environments without version macros
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
#else
    void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
#endif
#endif

    extern TxtBuffer<TXT_BUFFER_SIZE_PEER_FROM_BRIDGE> *peerRecvBuffer;
    extern TxtBuffer<TXT_BUFFER_SIZE_PEER_TO_BRIDGE> *peerTxBuffer;

    extern bool wifiInitialized;
    extern bool peerOrBridgeInitialized;
    extern bool isBridge;
    extern esp_now_peer_info_t peerInfo;

} // namespace ESPNOW
#endif
#endif