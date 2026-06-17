#include "Outgoing.h"

#include <limits.h>
#if !defined(ARDUINO_ARCH_AVR) && !defined(ARDUINO_ARCH_MEGAAVR) && !defined(ESP32) && !defined(ARDUINO_ARCH_ESP32) && !defined(ESP8266) && !defined(ARDUINO_ARCH_ESP8266)
#include <stdio.h>
#endif

#include "../System/BottangoCore.h"
#include "../../BottangoArduinoModules.h"

namespace Outgoing
{
	void printOutputStringPROGMEM(const char* targetOutput)
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

	void printOutputStringFlash(const __FlashStringHelper* str)
	{
#ifdef RELAY_SUPPORTED
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
#endif
	}

	void printOutputStringMem(const char* str)
	{
#ifdef RELAY_SUPPORTED
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
#endif
	}

	void printOutputStringMem(int value)
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

	void printOutputStringMem(long value)
	{
#if LONG_MAX == 2147483647L
		char buffer[12];
#elif LONG_MAX == 9223372036854775807L
		char buffer[21];
#else
		char buffer[21];
#endif
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(ESP32) || defined(ARDUINO_ARCH_ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
		ltoa(value, buffer, 10);
#else
		snprintf(buffer, sizeof(buffer), "%ld", value);
#endif
		printOutputStringMem(buffer);
	}

	void printOutputStringMem(char value)
	{
		char buffer[2];
		buffer[0] = value;
		buffer[1] = '\0';
		printOutputStringMem(buffer);
	}

	void printOutputStringMem(uint16_t value)
	{
		printOutputStringMem(static_cast<int>(value));
	}
	void printOutputStringMem(uint32_t value)
	{
		printOutputStringMem(static_cast<long>(value));
	}

	void printOutputStringMem(bool value)
	{
		printOutputStringFlash(value ? F("TRUE") : F("FALSE"));
	}

	void printLine()
	{
#ifdef RELAY_SUPPORTED
		if (BottangoCore::isRelayPeer)
		{
			if (secondaryPeerOutgoing)
			{
				Serial.print('\n');
			}
			else
			{
				if (BottangoCore::relayComs != nullptr)
				{
					BottangoCore::relayComs->peerPrintln();
				}
			}
		}
		else
		{
			Serial.print('\n');
		}
#elif defined(USE_USB_SERIAL)
		Serial.println();
#elif defined(USE_ESP32_WIFI)
		BottangoCore::client.println();
#endif
	}

	void outgoing_requestShutdown()
	{
		printOutputStringPROGMEM(REQ_SHUTDOWN);
	}

	void outgoing_requestStopPlay()
	{
		printOutputStringPROGMEM(STOP_PLAY);
	}

	void outgoing_requestStartPlay(int index, unsigned long time)
	{
		printOutputStringPROGMEM(START_PLAY);
		printOutputStringMem(String(index).c_str());
		printOutputStringFlash(F(","));
		printOutputStringMem(String(time).c_str());
		printLine();
	}

#ifdef ONLINE_BUTTON_ACTIONS
	void outgoing_requestStartPlayViaButton(int btnIdex)
	{
		printOutputStringPROGMEM(START_PLAY_BUTTON);
		printOutputStringMem(String(btnIdex).c_str());
		printLine();
	}
#endif

	void outgoing_requestStartPlay()
	{
		printOutputStringPROGMEM(START_PLAY);
		printOutputStringFlash(F("-1,-1"));
		printLine();
	}

	void outgoing_notifySyncComplete()
	{
		printOutputStringPROGMEM(SYNC_COMPLETE);
	}

	void flush()
	{
#ifdef RELAY_SUPPORTED
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
#endif
	}

#ifdef RELAY_SUPPORTED
	void setSecondaryPeerOutgoing(bool enabled)
	{
		secondaryPeerOutgoing = enabled;
	}
	void toggleOnSecondaryOutgoing()
	{
		if (secondaryPeerOutgoing)
		{
			ignoreToggleOff = true;
		}
		secondaryPeerOutgoing = true;
	}
	void endToggleOnSecondaryOutgoing()
	{
		if (ignoreToggleOff)
		{
			return;
		}
		secondaryPeerOutgoing = false;
	}
	bool ignoreToggleOff = false;
	bool secondaryPeerOutgoing = false;
#endif

} // namespace Outgoing
