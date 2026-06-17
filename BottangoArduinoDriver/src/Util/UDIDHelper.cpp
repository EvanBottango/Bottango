#include "UDIDHelper.h"

namespace UDIDHelper
{

#ifdef REPORT_UID
    void convertUIDToCStr(const uint8_t uid[UID_LENGTH], char str[UID_CSTR_SIZE])
    {
        for (size_t i = 0; i < UID_LENGTH; ++i)
        {
            // write two hex chars per byte
            sprintf(str + i * 2, "%02X", uid[i]);
        }
        // NULL terminate
        str[UID_HEX_LEN] = '\0';
    }
#endif

#ifdef RELAY_SUPPORTED
    bool convertCStrToMAC(const char *str, uint8_t mac[6])
    {
        if (str == nullptr)
        {
            for (int i = 0; i < 6; i++)
            {
                mac[i] = 0;
            }
            return false;
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
            return false;
        }

        // validate hex string before parsing
        for (int i = 0; i < len; i++)
        {
            char c = str[i];
            bool isHex = (c >= '0' && c <= '9') ||
                         (c >= 'a' && c <= 'f') ||
                         (c >= 'A' && c <= 'F');
            if (!isHex)
            {
                for (int j = 0; j < 6; j++)
                {
                    mac[j] = 0;
                }
                return false;
            }
        }

        // Convert each pair of hex characters into a byte.
        for (int i = 0; i < 6; i++)
        {
            char byteStr[3] = {str[2 * i], str[2 * i + 1], '\0'};
            mac[i] = (uint8_t)strtol(byteStr, NULL, 16);
        }

        return true;
    }

    void convertMACToCStr(const uint8_t mac[6], char *str)
    {
        // convert MAC bytes to a hex string in uppercase.
        sprintf(str, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    void getThisDeviceMacAddress(uint8_t mac[6])
    {
        esp_read_mac(mac, ESP_MAC_WIFI_STA); // == ESP_OK;
    }
#endif

} // namespace UDIDHelper
