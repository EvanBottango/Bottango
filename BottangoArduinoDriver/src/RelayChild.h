#include "../BottangoArduinoModules.h"
#if defined(RELAY_PARENT)

#ifndef RelayChild_h
#define RelayChild_h

#include <Arduino.h>
#include "../BottangoArduinoConfig.h"
#include "Time.h"
#include "TxtBuffer.h"

class RelayChild
{
public:
    RelayChild(char *identifier, char *macAddress);
    void getIdentifier(char *outArray, short arraySize);
    void passDownCommands(char *commands);
    void passUpCommands(char *commands);
    void destroy();
    void update();
    uint8_t mac_addr[6];

private:
    char identifier[9];

    TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> toChildBuffer;
    TxtBuffer<TXT_BUFFER_SIZE_ESPNOW> toParentBuffer;

    bool passUpNextOK = false;
    bool block = false;
};

#endif
#endif