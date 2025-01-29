#include "../BottangoArduinoModules.h"
#if defined(RELAY_PARENT)

#ifndef RelayChildPool_h
#define RelayChildPool_h

#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"

// Forward declaration of RelayChild
class RelayChild;

class RelayChildPool
{

public:
    RelayChildPool();

    void addRelay(RelayChild *relay);

    void removeRelay(char *identifier);

    void passThroughCommandToRelay(char *identifier, char **commands, byte commandCount);

    void deregisterAll();

    void update();

    RelayChild *getRelay(char *identifier);
    RelayChild *getRelay(const uint8_t *mac_addr);

private:
    bool isMacEqual(const uint8_t *mac1, const uint8_t *mac2);
    int hash(const char *str);
    // bool isMacEmpty(const uint8_t *mac);

    CircularArray<RelayChild> relays = CircularArray<RelayChild>(MAX_RELAY_CHILD);
};

#endif
#endif