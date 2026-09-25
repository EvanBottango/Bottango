#pragma once

#include "OutgoingR.h"

class OutgoingSerialImpl : public OutgoingBase
{
	protected:
	void printStringFlash_Implementation(const __FlashStringHelper* str) override;
	void printStringMem_Implementation(const char* str) override;
	void printLine_Implementation() override;
	void flush_Implementation() override;
};