#include "Relay.h"
#include "../../PersistentConfigUtil.h"
#include "../../System/SystemStatus.h"

void Relay::init()
{
	int relayState = PersistentConfigUtil::getRelayState();

	if (relayState == VALUE_RELAY_STATE_LIVE_USB)
	{
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
		//initUSBSerialComms();
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			Outgoing::printOutputStringFlash(F("relay state = usb"));
			Outgoing::printLine();
		}
#endif // RELAY_LOGGING
	}
	else if (relayState == VALUE_RELAY_STATE_BRIDGE)
	{
		_relayRole = RelayRole::Bridge;
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			Outgoing::printOutputStringFlash(F("relay state = bridge"));
			Outgoing::printLine();
		}
#endif // RELAY_LOGGING

		_relayPool = new RelayChildPool();
		ESPNowUtil::initializeESPNowAsBridge();

#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			Outgoing::printOutputStringFlash(F("bridge setup complete"));
			Outgoing::printLine();
		}
#endif // RELAY_LOGGING
	}
	else if (relayState == VALUE_RELAY_STATE_PEER)
	{
		//isRelayBridge = false;
		//isRelayPeer = true;
		_relayRole = RelayRole::Peer;
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Peer;

		secondaryCommandIdx = 0;
		secondaryCommandInProgress = false;
		secondaryTimeOfLastChar = 0;
		if (secondaryPeerCommandBuffer)
		{
			free(secondaryPeerCommandBuffer);
		}
		secondaryPeerCommandBuffer = (char*)malloc(MAX_COMMAND_LENGTH);

		//initUSBSerialComms();
#ifdef RELAY_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif // TOGGLE_DEBUG
		{
			Outgoing::toggleOnSecondaryOutgoing();
			Outgoing::printOutputStringFlash(F("relay state = peer. Sending BOOT"));
			Outgoing::printLine();
			Outgoing::endToggleOnSecondaryOutgoing();
			lastWaitForConnectLog = Time::getCurrentTimeInMs();
		}
#endif // RELAY_LOGGING

#ifdef RELAY_COMS_ESPNOW
		ESPNowUtil::initializeESPNowAsPeer();
		Outgoing::printLine();
		Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
		Outgoing::printLine();
#endif // RELAY_COMS_ESPNOW
	}
}