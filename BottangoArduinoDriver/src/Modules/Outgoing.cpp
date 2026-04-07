#include "Outgoing.h"
#include <limits.h>
#include "BasicCommands.h"

void OutgoingBase::printOutputStringPROGMEM(const char* targetOutput)
{
	char outputString[MAX_COMMAND_LENGTH];
	uint8_t iterator = 0;
	while (pgm_read_byte(targetOutput) != 0x00)
	{
		outputString[iterator] = (char)pgm_read_byte(targetOutput);
		targetOutput++;
		iterator++;
	}
	outputString[iterator] = '\0';
	printOutputStringMem(outputString);
}

void OutgoingBase::printOutputStringFlash(const __FlashStringHelper* str)
{
	printStringFlash_Implementation(str);

/*#ifdef RELAY_SUPPORTED
	if (BottangoCore::isRelayPeer)
	{
		if (secondaryPeerOutgoing)
		{
			Serial.print(str);
		}
		else
		{
			if (BottangoCore::relayComs != nullptr)
			{
				BottangoCore::relayComs->peerPrint(str);
			}
		}
	}
	else
	{
		Serial.print(str);
	}
#elif defined(USE_USB_SERIAL)
	Serial.print(str);
#elif defined(USE_ESP32_WIFI)
	BottangoCore::client.print(str);
#endif*/
}

void OutgoingBase::printOutputStringMem(const char* str)
{
	printStringMem_Implementation(str);
/*#ifdef RELAY_SUPPORTED
	if (BottangoCore::isRelayPeer)
	{
		if (secondaryPeerOutgoing)
		{
			Serial.print(str);
		}
		else
		{
			if (BottangoCore::relayComs != nullptr)
			{
				BottangoCore::relayComs->peerPrint(str);
			}
		}
	}
	else
	{
		Serial.print(str);
	}
#elif defined(USE_USB_SERIAL)
	Serial.print(str);
#elif defined(USE_ESP32_WIFI)
	BottangoCore::client.print(str);
#endif*/
}

void OutgoingBase::printOutputStringMem(int value)
{
#if INT_MAX == 2147483647
	char buffer[12];
#elif INT_MAX == 32767
	char buffer[7];
#else
	char buffer[12];
#endif
	itoa(value, buffer, 10);
	printOutputStringMem(buffer);
}

void OutgoingBase::printOutputStringMem(long value)
{
#if LONG_MAX == 2147483647L
	char buffer[12];
#elif LONG_MAX == 9223372036854775807L
	char buffer[21];
#else
	char buffer[21];
#endif
	ltoa(value, buffer, 10);
	printOutputStringMem(buffer);
}

void OutgoingBase::printOutputStringMem(char value)
{
	char buffer[2];
	buffer[0] = value;
	buffer[1] = '\0';
	printOutputStringMem(buffer);
}

void OutgoingBase::printOutputStringMem(uint16_t value)
{
	printOutputStringMem(static_cast<int>(value));
}

void OutgoingBase::printOutputStringMem(uint32_t value)
{
	printOutputStringMem(static_cast<long>(value));
}

void OutgoingBase::printOutputStringMem(bool value)
{
	printOutputStringFlash(value ? F("TRUE") : F("FALSE"));
}

void OutgoingBase::printLine()
{
	printLine_Implementation();
/*
#elif defined(USE_ESP32_WIFI)
	BottangoCore::client.println();
#endif*/
}

void OutgoingBase::outgoing_requestStartPlay(int index, unsigned long time)
{
	printOutputStringPROGMEM(BasicCommands::START_PLAY);
	printOutputStringMem(String(index).c_str());
	printOutputStringFlash(F(","));
	printOutputStringMem(String(time).c_str());
	printLine();
}

#ifdef ONLINE_BUTTON_ACTIONS
void OutgoingBase::outgoing_requestStartPlayViaButton(int btnIdex)
{
	printOutputStringPROGMEM(START_PLAY_BUTTON);
	printOutputStringMem(String(btnIdex).c_str());
	printLine();
}
#endif

void OutgoingBase::outgoing_requestStartPlay()
{
	printOutputStringPROGMEM(BasicCommands::START_PLAY);
	printOutputStringFlash(F("-1,-1"));
	printLine();
}

void OutgoingBase::flush()
{
	flush_Implementation();
/*#ifdef RELAY_SUPPORTED
	if (secondaryPeerOutgoing)
	{
		Serial.flush();
	}
	else
	{
		if (BottangoCore::relayComs != nullptr)
		{
			BottangoCore::relayComs->peerFlush();
		}
	}
#elif defined(USE_USB_SERIAL)
	Serial.flush();
#elif defined(USE_ESP32_WIFI)
	// BottangoCore::client.print(str);
#endif*/
}