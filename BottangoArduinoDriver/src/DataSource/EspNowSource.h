#pragma once

#include "../../BottangoArduinoModules.h"
#ifdef RELAY_COMS_ESPNOW

#include <Arduino.h>
#include "CharStreamedSource.h"

/**
 * @brief ESP Now data source. It uses ESP Now to read incoming commands. Commands are expected to be terminated with a newline character ('\n') and include a hash for validation.
 */
class EspNowSource : public CharStreamedSource
{
public:
	void onPhase(Phase const p) override;
	void init() override;
	void readData() override;
};

#endif // RELAY_COMS_ESPNOW