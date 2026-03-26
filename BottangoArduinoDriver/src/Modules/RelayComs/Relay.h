#pragma once

#include "../../../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED)

#include "RelayChildPool.h"
#include "../../Module Handling/ModuleLoop.h"

class Relay : public LoopModule
{
public:
	enum class RelayRole
	{
		Bridge,
		Peer
	};

	void onPhase(Phase p) override;
	void init() override;

	RelayRole getRole() const { return _relayRole; }

private:
	RelayChildPool* _relayPool = nullptr;
	RelayRole _relayRole = RelayRole::Peer;
};

#endif; // RELAY_SUPPORTED