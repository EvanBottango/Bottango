#include "../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)
#include "ESPNOWUtil.h"
#include "RelayChild.h"

namespace ESPNowUtil
{
    bool espNowInitialized = false;
    esp_now_peer_info_t peerInfo;

#if defined(RELAY_CHILD)
    TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> recvBuffer;
    TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> txBuffer;
    bool block = false;
#endif

    // Callback after data is sent
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
#ifdef RELAY_PARENT
        // ignore it for now?
        // TODO figure out what I need to do later
        if (status == ESP_NOW_SEND_SUCCESS)
        {
        }
        else
        {
        }
#elif defined(RELAY_CHILD)
        // also I guess just send for now?
#endif
    }

    // Callback when data is received
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
    {
        // Access the MAC address from recv_info->src_addr
        const uint8_t *mac_addr = recv_info->src_addr;
#else
    void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
    {
#endif
#endif
        char message[data_len + 1];
        memcpy(message, data, data_len);
        message[data_len] = '\0';

#ifdef RELAY_PARENT
        RelayChild *relay = BottangoCore::relayPool.getRelay(mac_addr);
        if (relay != nullptr)
        {
            relay->passUpCommands(message);
        }
#elif defined(RELAY_CHILD)
    block = true;
    if (recvBuffer.isFull())
    {
        // todo error here
        return;
    }
    else
    {
        recvBuffer.addTxt(message);
    }
    block = false;
#endif
    }

#ifdef RELAY_PARENT
    void initializeESPNow()
#elif defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
void initializeESPNow(const uint8_t *parent_mac_addr)
#endif
    {
        if (espNowInitialized)
        {
            return;
        }
        // Set device as a Wi-Fi Station
        WiFi.mode(WIFI_STA);

        // Init ESP-NOW
        if (esp_now_init() != ESP_OK)
        {
            Serial.println("Error initializing ESP-NOW"); // TODO use error here, not serial print
            return;
        }

        esp_now_register_send_cb(OnDataSent);   // register send callback
        esp_now_register_recv_cb((OnDataRecv)); // register recv callback

#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        memcpy(peerInfo.peer_addr, parent_mac_addr, 6);
        peerInfo.channel = ESPNOW_CHANNEL;
        peerInfo.encrypt = false;

        // Add parent peer
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add parent peer"); // TODO use error here, not serial print
            return;
        }
#endif
        espNowInitialized = true;
    }

#ifdef RELAY_PARENT
    void registerPeer(const uint8_t *mac_addr)
    {
        if (!espNowInitialized)
        {
            initializeESPNow();
        }

        memset(&peerInfo, 0, sizeof(esp_now_peer_info_t)); // Zero out the structure

        // Register peer
        memcpy(peerInfo.peer_addr, mac_addr, 6);
        peerInfo.channel = ESPNOW_CHANNEL;
        peerInfo.encrypt = false;

        // Add peer
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer"); // TODO use error here, not serial print
            return;
        }

        // peer added success
    }

    void deregisterPeer(const uint8_t *mac_addr)
    {
        // Deregister peer

        // Add peer
        if (esp_now_del_peer(mac_addr) != ESP_OK)
        {
            Serial.println("Failed to remove peer"); // TODO use error here, not serial print
            return;
        }
    }

    void sendCommand(const uint8_t *mac_addr, char *command)
    {
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(mac_addr, (uint8_t *)command, strlen(command));

        if (result != ESP_OK)
        {
            Serial.println("Error sending the data"); // TODO
        }
    }
#elif defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)

void print(const char *str)
{
    if (txBuffer.isFull())
    {
        // todo error here
        return;
    }

    // add to buffer
    txBuffer.addTxt(str);

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
        flush();
    }
}

void print(const __FlashStringHelper *str)
{
    char command[MAX_COMMAND_LENGTH];
    strncpy_P(command, (PGM_P)str, sizeof(command));
    command[sizeof(command) - 1] = '\0';
    print(command);
}

void println()
{
    print("\n");
}

void flush()
{
    while (txBuffer.available())
    {
        char message[MAX_COMMAND_LENGTH];
        txBuffer.getNextTxt(message);

        if (strlen(message) == 0)
        {
            // todo error here
            return;
        }

        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t *)message, strlen(message));

        if (result != ESP_OK)
        {
            Serial.print("Sent with failure "); // TODO
            Serial.println(message);
        }
    }
}

bool recvAvailable()
{
    return !block && recvBuffer.available();
}

void readRecv(char *output)
{
    recvBuffer.getNextTxt(output);
}

#endif
}
#endif