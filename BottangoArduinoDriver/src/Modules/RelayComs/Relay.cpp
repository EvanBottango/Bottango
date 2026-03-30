#include "Relay.h"
#include "../../PersistentConfigUtil.h"
#include "../../System/SystemStatus.h"
#include "BasicCommands.h"
#include "Modules/Outgoing.h"
#include "DataSource/EspNowSource.h"

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
		EspNowSource* secondarySource = BottangoCore::mMaster.registerModuleInSecondaryDataSlot<EspNowSource>();
		BottangoCore::mMaster.getModule<CommandDecoder>(Modules::Decoder)->setSecondaryDataSource(secondarySource);
	}
	else if (relayState == VALUE_RELAY_STATE_PEER)
	{		
		SystemStatus::systemStatus.ConnectionStatus = SystemStatus::eConnectionStatus::No_Connection_Peer;
		initializeAsPeer();
		EspNowSource* secondarySource = BottangoCore::mMaster.registerModuleInSecondaryDataSlot<EspNowSource>();
		BottangoCore::mMaster.getModule<CommandDecoder>(Modules::Decoder)->setSecondaryDataSource(secondarySource);

		// Change the default Outgoing messages to "Relay"
		Outgoing::bind(OutgoingRelay::get());

		Outgoing::printLine();
		Outgoing::printOutputStringPROGMEM(BasicCommands::BOOT);
		Outgoing::printLine();
	}
}