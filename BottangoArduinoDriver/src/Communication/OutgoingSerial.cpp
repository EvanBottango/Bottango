#include "OutgoingSerial.h"
#include "Arduino.h"

void OutgoingSerialImpl::printStringFlash_Implementation(const __FlashStringHelper* str)
{
	Serial.print(str);
}

void OutgoingSerialImpl::printStringMem_Implementation(const char* str)
{
	Serial.print(str);
}

void OutgoingSerialImpl::printLine_Implementation()
{
	Serial.println();
}

void OutgoingSerialImpl::flush_Implementation()
{
	Serial.flush();
}