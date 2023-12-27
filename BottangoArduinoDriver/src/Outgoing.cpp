#include <Arduino.h>

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

    void printOutputString(const char *targetOutput)
    {
        while (pgm_read_byte(targetOutput) != 0x00)
        {
            Serial.print((char)pgm_read_byte(targetOutput));
            targetOutput++;
        }
    }

    void outgoing_requestEStop()
    {
        printOutputString(ESTOP);
    }

    void outgoing_requestStopPlay()
    {
        printOutputString(STOP_PLAY);
    }

    void outgoing_requestStartPlay(int index, unsigned long time)
    {
        printOutputString(START_PLAY);
        Serial.print(index);
        Serial.print(F(","));
        Serial.print(time); // in MS
        Serial.print(F("\n"));
    }

    void outgoing_requestStartPlay()
    {
        printOutputString(START_PLAY);
        Serial.print(F("-1,-1\n"));
    }

    void outgoing_notifySyncComplete()
    {
        printOutputString(SYNC_COMPLETE);
    }

} // namespace Outgoing
