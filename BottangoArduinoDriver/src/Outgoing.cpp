#include <Arduino.h>
#include "BottangoCore.h"

#include "../BottangoArduinoModules.h"
#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
#include "ESPNOWUtil.h"
#endif

namespace Outgoing
{
    /** Request E Stop */
    const char ESTOP[] PROGMEM = "\nreqStop\n";

    /** Request pause anim */
    const char STOP_PLAY[] PROGMEM = "\nreqPause\n";

    /** Request start anim */
    const char START_PLAY[] PROGMEM = "\nreqPlay,";

    /** Stepper/Custom Motor Auto Sync is Complete */
    const char SYNC_COMPLETE[] PROGMEM = "\nsycMDone,";

    void printOutputStringPROGMEM(const char *targetOutput)
    {
        char outputString[MAX_COMMAND_LENGTH];
        uint8_t iterator = 0;
        while (pgm_read_byte(targetOutput) != 0x00)
        {
            outputString[iterator] = (char)pgm_read_byte(targetOutput);
            targetOutput++;
            iterator++;
        }
        outputString[iterator] = '\0';
        printOutputStringMem(outputString);
    }

    void printOutputStringFlash(const __FlashStringHelper *str)
    {

#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        ESPNowUtil::print(str);
#elif defined(USE_USB_SERIAL)
        Serial.print(str);
#elif defined(USE_ESP32_WIFI)
        BottangoCore::client.print(str);
#endif
    }

    void printOutputStringMem(const char *str)
    {
#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        ESPNowUtil::print(str);
#elif defined(USE_USB_SERIAL)
        Serial.print(str);
#elif defined(USE_ESP32_WIFI)
        BottangoCore::client.print(str);
#endif
    }

    void printOutputStringMem(int value)
    {
        char buffer[12];
        itoa(value, buffer, 10);
        printOutputStringMem(buffer);
    }

    void printOutputStringMem(long value)
    {
        char buffer[10];
        ltoa(value, buffer, 10);
        printOutputStringMem(buffer);
    }

    void printOutputStringMem(uint16_t value)
    {
        printOutputStringMem(static_cast<int>(value));
    }
    void printOutputStringMem(uint32_t value)
    {
        printOutputStringMem(static_cast<long>(value));
    }

    void printOutputStringMem(bool value)
    {
        printOutputStringFlash(value ? F("TRUE") : F("FALSE"));
    }

    void printLine()
    {
#if defined(RELAY_CHILD) && defined(RELAY_COMS_ESPNOW)
        ESPNowUtil::println();
#elif defined(USE_USB_SERIAL)
        Serial.println();
#elif defined(USE_ESP32_WIFI)
        BottangoCore::client.println();
#endif
    }

    void outgoing_requestEStop()
    {
        printOutputStringPROGMEM(ESTOP);
    }

    void outgoing_requestStopPlay()
    {
        printOutputStringPROGMEM(STOP_PLAY);
    }

    void outgoing_requestStartPlay(int index, unsigned long time)
    {
        printOutputStringPROGMEM(START_PLAY);
        printOutputStringMem(String(index).c_str());
        printOutputStringFlash(F(","));
        printOutputStringMem(String(time).c_str());
        printOutputStringFlash(F("\n"));
    }

    void outgoing_requestStartPlay()
    {
        printOutputStringPROGMEM(START_PLAY);
        printOutputStringFlash(F("-1,-1\n"));
    }

    void outgoing_notifySyncComplete()
    {
        printOutputStringPROGMEM(SYNC_COMPLETE);
    }

} // namespace Outgoing
