#include "BoardDefs.h"
#include "PersistentConfigUtil.h"
#include "BottangoCore.h"

namespace NamedBoardLoop
{

#ifdef NAMED_BOARD_LOOP

    unsigned long lastNamedButtonPressTime = 0;

    void runNamedBoardLoop()
    {
#if defined(BOTTANGO_NOVA) || defined(BOTTANGO_SOLAR)
        // we only care if in live connection mode
        if (BottangoCore::isOffline())
        {
            return;
        }
        // deal with debouncing
        if (millis() - lastNamedButtonPressTime < BUTTON_DEBOUNCE_TIME)
        {
            return;
        }

#ifdef BOTTANGO_NOVA
        bool btnAPressed = digitalRead(NAMED_BUTTON_A_PIN) == LOW;
        bool btnBPressed = digitalRead(NAMED_BUTTON_B_PIN) == LOW;
        bool btnCPressed = digitalRead(NAMED_BUTTON_C_PIN) == LOW;
#elif defined(BOTTANGO_SOLAR)
        int readVal = analogRead(NAMED_BUTTON_A_PIN);
        bool btnAPressed = readVal >= NAMED_BUTTON_A_READ_MIN && readVal <= NAMED_BUTTON_A_READ_MAX;
        bool btnBPressed = readVal >= NAMED_BUTTON_B_READ_MIN && readVal <= NAMED_BUTTON_B_READ_MAX;
        bool btnCPressed = readVal >= NAMED_BUTTON_C_READ_MIN && readVal <= NAMED_BUTTON_C_READ_MAX;
#endif

        if (btnAPressed || btnBPressed || btnCPressed)
        {
            int val = 0;
            if (btnBPressed)
            {
                val = 1;
            }
            else if (btnCPressed)
            {
                val = 2;
            }
            Outgoing::outgoing_requestStartPlayViaButton(val);
            lastNamedButtonPressTime = millis();
        }
#endif
    }

#endif

}