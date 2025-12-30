#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChild_h
#define RelayChild_h

#include <Arduino.h>
#include "../BottangoArduinoConfig.h"
#include "Time.h"
#include "TxtBuffer.h"

class RelayChild
{

public:
    RelayChild(char *macAddress);
    void passDownCommands(char *commands);
    void passUpCommands(char *commands);
    void destroy();
    void update();
    void requestBoot();
    uint8_t mac_addr[6];
    bool connected;

private:
    void onConnectionComplete();
    void onReboot();

    TxtBuffer<TXT_BUFFER_SIZE_RX_FROM_PEER> incomingFromPeerBuffer;
    char disconnectedMessageHoldingBuffer[MAX_COMMAND_LENGTH] = {0};
    unsigned long lastMsgTime = 0;
    bool heartbeatOutstanding = false;
};

#endif
#endif