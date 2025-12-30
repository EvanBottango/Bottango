#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED) && defined(RELAY_COMS_ESPNOW)

#ifndef MacResponder_h
#define MacResponder_h

#include "AbstractMultiMessageOutgoingSource.h"

// -> rMAC
// <- OK
// <- sMAC,FFFFFFFFFFFF
// -> OK
// Complete

const char REPLY_MAC_ADDRESS[] PROGMEM = "sMAC";

class MACResponder : public AbstractMultiMessageOutgoingSource
{
    virtual void initializeMultiMessage() override;

    virtual bool multiMessageisComplete() override; // when everything is done and responded to, ready to clean up

    virtual void updateMultiMessage() override; // will send if anything to send, will timeout if waiting and no response, will send closing if ready/any

    virtual void cleanUpMultiMessage() override; // cleanup if aborting...

    bool macSent = false;
};

#endif
#endif