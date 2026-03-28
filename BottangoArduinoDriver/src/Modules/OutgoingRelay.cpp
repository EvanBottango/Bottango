#include "../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#include "OutgoingRelay.h"

void OutgoingRelayImpl::printStringFlash_Implementation(const __FlashStringHelper* str)
{
	_relayComs->peerPrint(str);
}

void OutgoingRelayImpl::printStringMem_Implementation(const char* str)
{
	_relayComs->peerPrint(str);
}

void OutgoingRelayImpl::printLine_Implementation()
{
	_relayComs->peerPrintln();
}

void OutgoingRelayImpl::flush_Implementation()
{
	_relayComs->peerFlush();
}
#endif // RELAY_SUPPORTED