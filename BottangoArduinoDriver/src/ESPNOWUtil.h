#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#ifndef ESPNowUtil_h
#define ESPNowUtil_h

#include "Arduino.h"
#include <esp_now.h>
#include <WiFi.h>
#include "BottangoCore.h"
#include "TxtBuffer.h"

namespace ESPNowUtil
{
#ifdef RELAY_PARENT
    void initializeESPNow();
    void registerPeer(const uint8_t *mac_addr);
    void deregisterPeer(const uint8_t *mac_addr);
    void sendCommand(const uint8_t *mac_addr, char *command);
#elif defined(RELAY_CHILD)
    void initializeESPNow(const uint8_t *parent_mac_addr);
    void print(const char *str);
    void print(const __FlashStringHelper *str);
    void println();
    void flush();
    bool recvAvailable();
    void readRecv(char *output);
#endif

    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
#else
    void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
#endif
#endif

#if defined(RELAY_CHILD)
    extern TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> recvBuffer;
    extern TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> txBuffer;
#endif

    extern bool espNowInitialized;
    extern esp_now_peer_info_t peerInfo;
} // namespace ESPNOW
#endif
#endif