#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChild_h
#define RelayChild_h

#include <Arduino.h>
#include "../BottangoArduinoConfig.h"
#include "Time.h"
#include "TxtBuffer.h"
#include "RelayChildMessageQueue.h"

class RelayChild
{

public:
    RelayChild(char *macAddress);
    void passDownCommands(char *commands, MessageIntent intent = MessageIntent::Normal);
    void passUpCommands(char *commands);
    void destroy();
    void update();
    void markTxTime();
    void markPollOutstanding();
    void clearPollOutstanding();
    bool pollOutstandingAndExpired(unsigned long now, unsigned long timeout);
    int stableId = -1;
    uint8_t mac_addr[6];
    bool connected = false;
    bool teardown = false;
    bool teardownReadyToFinalize = false;

private:
    void onConnectionComplete();
    void onReboot();

    TxtBuffer<TXT_BUFFER_SIZE_RX_FROM_PEER> incomingFromPeerBuffer;
    char singleMessageHoldingBuffer[MAX_COMMAND_LENGTH] = {0};
    MessageIntent singleMessageHoldingIntent = MessageIntent::Normal;
    unsigned long lastTxTime = 0;
    bool pollOutstanding = false;
};

#endif
#endif
