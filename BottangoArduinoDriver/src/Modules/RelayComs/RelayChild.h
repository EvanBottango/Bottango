#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#ifndef RelayChild_h
#define RelayChild_h

#include <Arduino.h>
#include "../BottangoArduinoConfig.h"
#include "System/Time.h"
#include "DataSource/TxtBuffer.h"

class RelayChild
{
public:
	int stableId = -1;
	uint8_t mac_addr[6];
	bool connected;

	RelayChild(char* macAddress);
	void passDownCommands(char* commands);
	void passUpCommands(char* commands);
	void destroy();
	void update();
	void markTxTime();
	void markPollOutstanding();
	void clearPollOutstanding();
	bool pollOutstandingAndExpired(unsigned long now, unsigned long timeout);

private:
	TxtBuffer<TXT_BUFFER_SIZE_RX_FROM_PEER> _incomingFromPeerBuffer;
	char _disconnectedMessageHoldingBuffer[MAX_COMMAND_LENGTH] = { 0 };
	unsigned long _lastTxTime = 0;
	bool _pollOutstanding = false;

	void onConnectionComplete();
	void onReboot();
};

#endif // RelayChild_h
#endif // RELAY_SUPPORTED