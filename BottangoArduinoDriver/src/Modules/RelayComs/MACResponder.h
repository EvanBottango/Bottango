#include "../BottangoArduinoModules.h"
#ifdef RELAY_SUPPORTED

#ifndef MacResponder_h
#define MacResponder_h

#include "AbstractMultiMessageOutgoingSource.h"

// -> rMAC
// <- OK
// <- sMAC,FFFFFFFFFFFF
// -> OK
// <- OK
// Complete

const char REPLY_MAC_ADDRESS[] PROGMEM = "sMAC";

class MACResponder : public AbstractMultiMessageOutgoingSource
{
public:
	virtual void cleanUpMultiMessage() override; // cleanup if aborting...

private:
	void onMultiMessageStart() override;
	bool emitNextChunk() override;

};

#endif // MacResponder_h
#endif // RELAY_SUPPORTED