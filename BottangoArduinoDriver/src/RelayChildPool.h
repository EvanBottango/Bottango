#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChildPool_h
#define RelayChildPool_h

#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"
#include "AbstractMultiMessageOutgoingSource.h"
#include "RelayChildMessageQueue.h"

// Forward declaration of RelayChild
class RelayChild;

const char RELAY_ID_RESPONSE_PREFIX[] PROGMEM = "rlyId,";

class RelayChildPool : public AbstractMultiMessageOutgoingSource
{

public:
    RelayChildPool();

    void addRelay(RelayChild *relay);

    void removeRelay(int id);

    void passThroughCommandToRelay(int id, char **commands, byte paramsCount);

    void deregisterAll();

    void update();

    bool bridgeIsConnectedToAllPeers();

    void sendStopTimeCommand(RelayChild *peer);
    void sendHeartbeat(RelayChild *peer);
    void resumeTimeConnectedPeers(bool clearCurves);
    void stopTimeOnConnectedPeers();

    void sendHandshakeCommand(RelayChild *peer);

    void clearCurvesOnConnectedPeers();
    void sendClearCurvesCommand(RelayChild *peer);

    RelayChild *getRelay(int id);
    RelayChild *getRelay(const uint8_t *mac_addr);
    int getIdForRelay(RelayChild *relayChild);

    bool toPeerQueueFull() const { return toPeerQueue.full(); }

    virtual void initializeMultiMessage() override; // setup

    virtual bool multiMessageisComplete() override; // when everything is done and responded to, ready to clean up

    virtual void updateMultiMessage() override; // will send if anything to send, will timeout if waiting and no response, will send closing if ready/any

    virtual void cleanUpMultiMessage() override;

    bool enqueueToSendQueue(RelayChild *peer, char *commandString);

    RelayChildMessageQueue &outgoingQueue() { return toPeerQueue; }

private:
    bool isMacEqual(const uint8_t *mac1, const uint8_t *mac2);
    int hash(const char *str);

    void executePassThrough(RelayChild *peer, char *commandString);

    RelayChildMessageQueue toPeerQueue;

    enum OutgoingMessageStatus
    {
        idle,
        sendID,
        waitingForContinue,
        complete
    };
    int relayIdToReport = -1;
    OutgoingMessageStatus messageStatus;

    CircularArray<RelayChild>
        relays = CircularArray<RelayChild>(MAX_RELAY_CHILD);
};

#endif
#endif