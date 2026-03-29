#include "Relay.h"
#include "../../PersistentConfigUtil.h"
#include "../../System/SystemStatus.h"
#include "BasicCommands.h"
#include "Modules/Outgoing.h"

void Relay::init()
{
	int relayState = PersistentConfigUtil::getRelayState();

	if (relayState == VALUE_RELAY_STATE_LIVE_USB)
	{
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;
		_relayRole = RelayRole::None;
	}
	else if (relayState == VALUE_RELAY_STATE_BRIDGE)
	{
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Serial;

		_relayPool = new RelayChildPool();
		initializeAsBridge();
	}
	else if (relayState == VALUE_RELAY_STATE_PEER)
	{		
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Peer;
		initializeAsPeer();

		// Change the default Outgoing messages to "Relay"
		Outgoing::bind(OutgoingRelay::get());

		Outgoing::printLine();
		Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
		Outgoing::printLine();
	}
}