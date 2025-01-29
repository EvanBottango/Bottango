#include "../BottangoArduinoModules.h"
#include "OTAUpdateUtil.h"

#if defined(ENABLE_ESP_OTA_UPDATE)
#include "Outgoing.h"
#include "BasicCommands.h"

namespace OTAUpdateUtil
{
    bool otaInProgress = false;
    uint32_t checksum = 0;

    void beginOTA()
    {
        if (otaInProgress)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-InProgress"));
            return;
        }

        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-InitFail"));
            return;
        }

        otaInProgress = true;
        checksum = 0;
    }

    void recvOTAData(const char *hexData)
    {
        if (!otaInProgress)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-NotStarted"));
            return;
        }

        int len = strlen(hexData);
        if (len % 2 != 0)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-BadHex"));
            return;
        }

        int dataLen = len / 2;
        uint8_t *buffer = (uint8_t *)malloc(dataLen);
        if (!buffer)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-MemAlloc"));
            return;
        }

        // Convert hex string to binary data
        for (int i = 0; i < dataLen; i++)
        {
            char c1 = hexData[i * 2];
            char c2 = hexData[i * 2 + 1];
            char temp[3] = {c1, c2, '\0'};
            buffer[i] = (uint8_t)strtol(temp, NULL, 16);
        }

        // Write the chunk to flash
        size_t written = Update.write(buffer, dataLen);
        if (written != (size_t)dataLen)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-WriteFail"));
            free(buffer);
            return;
        }

        // Update checksum
        for (int i = 0; i < dataLen; i++)
        {
            checksum += buffer[i];
        }

        free(buffer);
    }

    void finishOTA(const char *expectedChecksumStr)
    {
        if (!otaInProgress)
        {
            // error here
            Outgoing::printOutputStringFlash(F("ERR: OTA-NotStarted"));
            return;
        }

        uint32_t expectedChecksum = (uint32_t)strtoul(expectedChecksumStr, NULL, 10);

        if (checksum == expectedChecksum)
        {
            // Finalize the update
            if (Update.end(true))
            {
                otaInProgress = false;
                Outgoing::printOutputStringFlash(F("OTA Success, Restarting"));
                Outgoing::printLine();
                Outgoing::printOutputStringPROGMEM(BasicCommands::READY);

                // yes, delay on purpose. Blocking the thread is desired here, I don't want anything else to come in
                // this should be the only place delay is actually correct in this codebase...
                delay(1000);
                ESP.restart();
            }
            else
            {
                Outgoing::printOutputStringFlash(F("ERR: OTA-EndFail"));
                otaInProgress = false;
                Update.abort();
            }
        }
        else
        {
            Outgoing::printOutputStringFlash(F("WARN: OTA-BadChecksum"));
            otaInProgress = false;
            Update.abort();
        }
    }
}
#endif