#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)
#include "ESPNOWUtil.h"
#include "RelayChild.h"
#include <stdio.h>

#ifdef RELAY_LOGGING
#include "esp_err.h"
#endif

namespace ESPNowUtil
{
    bool peerOrBridgeInitialized = false;
    bool wifiInitialized = false;
    bool netifReady = false;

    esp_now_peer_info_t peerInfo;

    TxtBuffer<TXT_BUFFER_SIZE_PEER_FROM_BRIDGE> *peerRecvBuffer = nullptr;
    TxtBuffer<TXT_BUFFER_SIZE_PEER_TO_BRIDGE> *peerTxBuffer = nullptr;
    char *peerRecvReadBuffer;
    bool block = false;
    bool isBridge = false;

    // Callback after data is sent
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // Arduino-ESP32 3.x (IDF 5.x): first param is wifi_tx_info_t*
    void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
#else
    // Arduino-ESP32 2.x and earlier
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
#else
    // Fallback for environments without version macros
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
    {
        //         if (status != ESP_NOW_SEND_SUCCESS)
        //         {
        // #ifdef RELAY_LOGGING
        // #ifdef TOGGLE_DEBUG
        //             if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
        // #endif
        //             {
        //                 Outgoing::toggleOnSecondaryOutgoing();
        //                 Outgoing::printOutputStringFlash(F("Fail tx espnow"));
        //                 Outgoing::printLine();
        //                 Outgoing::endToggleOnSecondaryOutgoing();
        //             }
        // #endif
        //         }
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

        if (isBridge)
        {
            RelayChild *relay = BottangoCore::relayPool->getRelay(mac_addr);
            if (relay != nullptr)
            {
                relay->passUpCommands(message);
            }
#ifdef RELAY_LOGGING
            else
            {
#ifdef TOGGLE_DEBUG
                if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
                {
                    Outgoing::toggleOnSecondaryOutgoing();
                    Outgoing::printOutputStringFlash(F("RCV msg "));
                    Outgoing::printOutputStringMem(message);
                    Outgoing::printOutputStringFlash(F(" from unknown peer "));
                    char cMAC[20];
                    convertMacToCStr(mac_addr, cMAC);
                    Outgoing::printOutputStringMem(cMAC);
                    Outgoing::printLine();
                    Outgoing::endToggleOnSecondaryOutgoing();
                }
            }
#endif
        }
        else
        {
            block = true;
            if (peerRecvBuffer && peerRecvBuffer->isFull())
            {
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
                if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
                {
                    Outgoing::toggleOnSecondaryOutgoing();
                    Outgoing::printOutputStringFlash(F("Msg recieved, in buffer full: "));
                    Outgoing::printOutputStringMem(message);
                    Outgoing::printLine();
                    Outgoing::endToggleOnSecondaryOutgoing();
                }
#endif
                return;
            }
            else
            {
                if (peerRecvBuffer)
                {
                    peerRecvBuffer->addTxt(message);
                }
            }
            block = false;
        }
    }

    static void espNowToPeerSendTask(void *pvParameters)
    {
        RelayChildPool *pool = static_cast<RelayChildPool *>(pvParameters);
        RelayChildMessageQueue &queue = pool->outgoingQueue();
        OutgoingMessage msg;

        for (;;)
        {
            if (queue.peek(msg))
            {
                esp_err_t res = esp_now_send(msg.mac,
                                             reinterpret_cast<const uint8_t *>(msg.payload),
                                             msg.length);

                if (res == ESP_OK)
                {
                    queue.pop();
                }
                else
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
            else
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }

    void initializeESPNowAsBridge()
    {
        if (peerOrBridgeInitialized)
        {
            return;
        }
        initESPNowConnection();

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
            espNowToPeerSendTask,
            "espNowPeerSend",
            2048,
            BottangoCore::relayPool, // pass the pool pointer
            1,
            nullptr);

        peerOrBridgeInitialized = true;
        isBridge = true;
    }

    void initializeESPNowAsPeer()
    {
        if (peerOrBridgeInitialized)
        {
            return;
        }

        peerRecvBuffer = new TxtBuffer<TXT_BUFFER_SIZE_PEER_FROM_BRIDGE>();
        peerTxBuffer = new TxtBuffer<TXT_BUFFER_SIZE_PEER_TO_BRIDGE>();

        initESPNowConnection();

        uint8_t mac[6];
        bool gotBridgeMac = PersistentConfigUtil::getBridgeMacAddress(mac);
        if (!gotBridgeMac)
        {
            Outgoing::printOutputStringFlash(F("ERR: No bridge mac for peer"));
            Outgoing::printLine();
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
            convertMacToCStr(mac, macC);
            Outgoing::toggleOnSecondaryOutgoing();
            Outgoing::printOutputStringFlash(F("Peer callback init from bridge with MAC "));
            Outgoing::printOutputStringMem(macC);
            Outgoing::printLine();
            Outgoing::endToggleOnSecondaryOutgoing();
        }

#endif
        peerOrBridgeInitialized = true;
        isBridge = false;
    }

    void initESPNowConnection()
    {
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

        esp_now_register_send_cb(OnDataSent); // register send callback
        esp_now_register_recv_cb(OnDataRecv); // register recv callback

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

    void getThisDeviceMACAddress(uint8_t mac[6])
    {
        if (!wifiInitialized)
        {
            initESPNowConnection();
        }
        // WiFi.macAddress(mac);
        esp_wifi_get_mac(WIFI_IF_STA, mac);
    }

    void convertCStrToMac(const char *str, uint8_t mac[6])
    {
        if (str == NULL)
        {
            for (int i = 0; i < 6; i++)
            {
                mac[i] = 0;
            }
            return;
        }

        // check length (ignore trailing newline, etc.)
        int len = strlen(str);
        while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
        {
            len--;
        }
        if (len != 12)
        {
            for (int i = 0; i < 6; i++)
            {
                mac[i] = 0;
            }
            return;
        }

        // Convert each pair of hex characters into a byte.
        for (int i = 0; i < 6; i++)
        {
            char byteStr[3] = {str[2 * i], str[2 * i + 1], '\0'};
            mac[i] = (uint8_t)strtol(byteStr, NULL, 16);
        }
    }

    void convertMacToCStr(const uint8_t mac[6], char *str)
    {
        // convert MAC bytes to a hex string in uppercase.
        sprintf(str, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    void registerPeer(const uint8_t *mac_addr)
    {
        if (!peerOrBridgeInitialized)
        {
            initializeESPNowAsBridge();
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

    void deregisterPeer(const uint8_t *mac_addr)
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

    void peerPrint(const char *str)
    {
        if (peerTxBuffer && peerTxBuffer->isFull())
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
        if (peerTxBuffer)
        {
            peerTxBuffer->addTxt(str);
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

    void peerPrint(const __FlashStringHelper *str)
    {
        char command[MAX_COMMAND_LENGTH];
        strncpy_P(command, (PGM_P)str, sizeof(command));
        command[sizeof(command) - 1] = '\0';
        peerPrint(command);
    }

    void peerPrintln()
    {
        peerPrint("\n");
    }

    void peerFlush()
    {
        while (peerTxBuffer && peerTxBuffer->available())
        {
            char message[MAX_COMMAND_LENGTH];
            peerTxBuffer->getNextTxt(message);

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
            esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t *)message, strlen(message));

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

    bool peerRecvAvailable()
    {
        return !block && peerRecvBuffer && peerRecvBuffer->available();
    }

    char peerReadNextChar()
    {
        return peerRecvBuffer ? peerRecvBuffer->getNextChar() : '\0';
    }

}
#endif