#pragma once

#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#include "Outgoing.h"
#include "RelayComs/Relay.h"

class OutgoingRelayImpl : public OutgoingBase
{
public:
	void setRelayComs(Relay* relayComs) { _relayComs = relayComs; }

protected:
	void printStringFlash_Implementation(const __FlashStringHelper* str) override;
	void printStringMem_Implementation(const char* str) override;
	void printLine_Implementation() override;
	void flush_Implementation() override;

private:
	Relay* _relayComs = nullptr;
};
#endif // RELAY_SUPPORTED