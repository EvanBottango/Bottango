#ifndef AbstractMultiMessageOutgoingSource_h
#define AbstractMultiMessageOutgoingSource_h

#include <Arduino.h>
#include "../BottangoArduinoModules.h"

class AbstractMultiMessageOutgoingSource
{
public:
    void setRecievedContinue();

#ifdef RELAY_SUPPORTED
    void setSecondary();
#endif

    virtual void initializeMultiMessage() = 0; // setup

    virtual bool multiMessageisComplete() = 0; // when everything is done and responded to, ready to clean up

    virtual void updateMultiMessage() = 0; // will send if anything to send, will timeout if waiting and no response, will send closing if ready/any

    virtual void cleanUpMultiMessage() = 0; // cleanup if aborting...

    bool hasOutgoingMessage;

protected:
    bool isTimeout();
    void setTransmitted();
    unsigned long lastMessageTime = 0;
#ifdef RELAY_SUPPORTED
    bool secondary = false;
#endif
};

#endif