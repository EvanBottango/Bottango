#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef RELAY_SUPPORTED

#include "OutgoingR.h"
#include "RelayComs/IRelayComms.h"

class OutgoingRelayImpl : public OutgoingBase
{
public:
	void setRelayComs(IRelayComms* relayComs) { _relayComs = relayComs; }

protected:
	void printStringFlash_Implementation(const __FlashStringHelper* str) override;
	void printStringMem_Implementation(const char* str) override;
	void printLine_Implementation() override;
	void flush_Implementation() override;

private:
	IRelayComms* _relayComs = nullptr;
};
#endif // RELAY_SUPPORTED