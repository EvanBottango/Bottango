#pragma once

#include "../BottangoArduinoModules.h"

#ifdef RELAY_SUPPORTED
#include <Arduino.h>

struct IRelayComms
{
    virtual ~IRelayComms() = default;

    // Bridge
    virtual void initializeAsBridge() = 0;
    virtual void registerPeer(const uint8_t *udid) = 0;
    virtual void deregisterPeer(const uint8_t *udid) = 0;

    // Peer
    virtual void initializeAsPeer() = 0;
    virtual void peerPrint(const char *str) = 0;
    virtual void peerPrint(const __FlashStringHelper *str) = 0;
    virtual void peerPrintln() = 0;
    virtual void peerFlush() = 0;
    virtual bool peerRecvAvailable() = 0;
    virtual char peerReadNextChar() = 0;

    // Shared utilities / lifecycle
    virtual bool getIsBridge() = 0;
    virtual void update() = 0;
};
#endif
