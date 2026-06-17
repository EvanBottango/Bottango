#include <limits.h>
#include "ModulesResponder.h"
#include "../Communication/Outgoing.h"
#include "../System/BottangoCore.h"

void ModulesResponder::onMultiMessageStart()
{
	iterator = 0;
}

bool ModulesResponder::emitNextChunk()
{
#ifdef RELAY_SUPPORTED
	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(true);
	}
#endif

	// send out modules response based on what's supported
	while (iterator < MODULES_COUNT)
	{
		bool didSendResponse = false;
		switch (iterator)
		{
			// UID Reporting
		case 0:
			didSendResponse = sendUIDResponse();
			break;

			// Named Board
		case 1:
			didSendResponse = sendNamedBoardResponse();
			break;

			// Command source
		case 2:
			didSendResponse = sendCommandSourceResponse();
			break;

			// Command configuration
		case 3:
			didSendResponse = sendCommandConfigResponse();
			break;

			// Relay Support
		case 4:
			didSendResponse = sendRelayResponse();
			break;

			// pca driver
		case 5:
			didSendResponse = sendPCAResponse();
			break;

			// i2s audio
		case 6:
			didSendResponse = sendI2SResponse();
			break;

			// stop Button
		case 7:
			didSendResponse = sendStopButtonResponse();
			break;

		default:
			didSendResponse = false;
		}

		iterator++;

		if (didSendResponse)
		{
#ifdef RELAY_SUPPORTED
			if (secondary)
			{
				Outgoing::setSecondaryPeerOutgoing(false);
			}
#endif
			return true;
		}
	}

	if (iterator == MODULES_COUNT)
	{
		sendClosingModuleResponse();
		iterator++; // Move past MODULES_COUNT to mark completion
#ifdef RELAY_SUPPORTED
		if (secondary)
		{
			Outgoing::setSecondaryPeerOutgoing(false);
		}
#endif
		return true;
	}

#ifdef RELAY_SUPPORTED
	if (secondary)
	{
		Outgoing::setSecondaryPeerOutgoing(false);
	}
#endif
	return false;
}

// !! different module response methods !! //
void ModulesResponder::sendClosingModuleResponse()
{
	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(END_OF_MODULES);           // MOD,EoM
	Outgoing::printLine();
}

bool ModulesResponder::sendUIDResponse()
{
#ifdef REPORT_UID
	byte uid[UID_LENGTH];
	PersistentConfigUtil::getUID(uid);
	char uidBuffer[UID_CSTR_SIZE];
	UDIDHelper::convertUIDToCStr(uid, uidBuffer);

	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(UID_PREFIX);               // MOD,UID
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,UID,
	Outgoing::printOutputStringMem(uidBuffer);                    // MOD,UID,xxxxxxxx
	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

bool ModulesResponder::sendCommandSourceResponse()
{
	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(COMMAND_SOURCE_PREFIX);    // MOD,CMD_SRC
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,CMD_SRC,
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
	if (PersistentConfigUtil::getUseExportedCommandStream())
	{
		Outgoing::printOutputStringMem(3); // MOD,CMD_SRC,3 == Toggleable, Export
	}
	else
	{
		Outgoing::printOutputStringMem(2); // MOD,CMD_SRC,2 == Toggleable, Live
	}
#else
	Outgoing::printOutputStringMem(1); // MOD,CMD_SRC,1 == Export Only Mode
#endif
#else
	Outgoing::printOutputStringMem(0); // MOD,CMD_SRC,0 == Live Only Mode
#endif
	Outgoing::printLine();

	return true;
}

bool ModulesResponder::sendCommandConfigResponse()
{
	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(COMMAND_CONFIG_PREFIX);    // MOD,CMD_CFG
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,CMD_CFG,

	// max message length
	Outgoing::printOutputStringMem(MAX_COMMAND_LENGTH);
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,CMD_CFG,(max message length),

	// max buffered curves
	Outgoing::printOutputStringMem(MAX_NUM_CURVES);
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,CMD_CFG,(max message length),(max buffered curve count)
	// sync curves?
#ifdef ALLOW_SYNC_COMMANDS
	Outgoing::printOutputStringMem(1);
#else
	Outgoing::printOutputStringMem(0);
#endif
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,CMD_CFG,(max message length),(max buffered curve count),(allow sync curves),

	// max motor signal size
#if INT_MAX == 0x7FFF // 2¹⁵−1
	Outgoing::printOutputStringMem(16);
#elif INT_MAX == 0x7FFFFFFF // 2³¹−1
	Outgoing::printOutputStringMem(32);
#endif
	// MOD,CMD_CFG,(max message length),(max buffered curve count),(allow sync curves),(max motor signal size)

	Outgoing::printLine();
	return true;
}

bool ModulesResponder::sendPCAResponse()
{
#ifdef USE_ADAFRUIT_PWM_LIBRARY
	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(PCA_PREFIX);               // MOD,9685
	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

bool ModulesResponder::sendI2SResponse()
{
#ifdef AUDIO_SD_I2S
	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(AUDIO_I2S_PREFIX);         // MOD,I2S
	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

bool ModulesResponder::sendStopButtonResponse()
{

#ifdef STOP_BUTTON_SUPPORTED
#ifndef DYNAMIC_STOP_BUTTON_BEHAVIOR
	bool stopIsShutdown = STOP_BUTTON_SHOULD_DISCONNECT;
	bool stopIsDynamic = false;
#else
	bool stopIsShutdown = PersistentConfigUtil::stopIsShutdown();
	bool stopIsDynamic = true;
#endif

	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(STOP_BTN_PREFIX);          // MOD,STP_BTN
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,STP_BTN,
	Outgoing::printOutputStringMem((int)stopIsShutdown);          // MOD,STP_BTN,0
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,STP_BTN,0,
	Outgoing::printOutputStringMem((int)stopIsDynamic);           // MOD,STP_BTN,0,0
	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

bool ModulesResponder::sendNamedBoardResponse()
{
#ifdef NAMED_BOARD
	char hwVerBuffer[NAMED_BOARD_HW_VER_LENGTH];
	PersistentConfigUtil::getNamedHWVersion(hwVerBuffer);

	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(NAMED_BOARD_PREFIX);       // MOD,BOARD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,BOARD,
	Outgoing::printOutputStringPROGMEM(NAMED_BOARD_MODEL);        // MOD,BOARD,X(Solar,Impulse,Nova)
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,BOARD,X(Solar,Impulse,Nova),
	Outgoing::printOutputStringMem(hwVerBuffer);                  // MOD,BOARD,X(Solar,Impulse,Nova),Y(1.3, etc)
	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

bool ModulesResponder::sendRelayResponse()
{
#ifdef RELAY_SUPPORTED
	int relayTypeVal = VALUE_RELAY_STATE_LIVE_USB;
	if (BottangoCore::isRelayBridge)
	{
		relayTypeVal = VALUE_RELAY_STATE_BRIDGE;
	}
	else if (BottangoCore::isRelayPeer)
	{
		relayTypeVal = VALUE_RELAY_STATE_PEER;
	}

	Outgoing::printOutputStringPROGMEM(MODULES_RESPONSE_PREFIX);  // MOD
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,
	Outgoing::printOutputStringPROGMEM(RELAY_PREFIX);             // MOD,RLY
	Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,RLY,
	Outgoing::printOutputStringMem(relayTypeVal);                 // MOD,RLY,2
	if (BottangoCore::isRelayPeer || BottangoCore::isRelayBridge)
	{
		Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,RLY,2,
		uint8_t mac[6];
		char macStr[20];

		// peer and bridge returns this device Mac
		PersistentConfigUtil::getThisDeviceMacAddress(mac);
		UDIDHelper::convertMACToCStr(mac, macStr);
		Outgoing::printOutputStringMem(macStr); // MOD,RLY,2,thisMac

		// peer returns bridge mac
		if (BottangoCore::isRelayPeer)
		{
			Outgoing::printOutputStringPROGMEM(MODULES_PARAM_DELINIATOR); // MOD,RLY,2,thisMac,
			bool gotBridgeMac = PersistentConfigUtil::getBridgeMacAddress(mac);
			UDIDHelper::convertMACToCStr(mac, macStr);
			Outgoing::printOutputStringMem(macStr); // MOD,RLY,2,thisMac,bridgeMac
		}
	}

	Outgoing::printLine();
	return true;
#else
	return false;
#endif
}

void ModulesResponder::cleanUpMultiMessage()
{
	delete this;
}
