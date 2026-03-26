#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(RELAY_COMS_ESPNOW)

#include <Arduino.h>
#include "CharStreamedSource.h"

class EspNowSource : public CharStreamedSource
{
	void onPhase(Phase p) override;
	void init() override;
	void readData() override;
};

#endif // RELAY_COMS_ESPNOW