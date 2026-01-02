/**
 * @defgroup ModuleSystem Module master system
 * @{
 */

#include "ModuleMaster.h"
#include "../../BottangoArduinoModules.h"
#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLightsModule.h"
#include "../Modules/Audio/I2SAudioModule.h"
#include "../Outgoing.h"

void ModuleMaster::setupModules()
{
#ifdef STOP_BUTTON_SUPPORTED
	static StopButtonModule stopButton;
	modules[(int)Modules::StopButton] = &stopButton;
#endif

#ifdef ENABLE_STATUS_LIGHTS
	static StatusLightsModule statusLights;
	modules[(int)Modules::StatusLights] = &statusLights;
#endif

#ifdef AUDIO_SD_I2S
	static I2SAudioModule audioModule;
	modules[(int)Modules::AudioI2S] = &audioModule;
	InterfaceRegistry::registerInterface(Modules::AudioI2S, static_cast<IAudioPlayback*>(&audioModule));
#endif
}

void ModuleMaster::initModules()
{
	for (int i = 0; i < (int)Modules::Max; i++)
	{
		modules[i]->init();
	}
}

void ModuleMaster::executePhase(Phase p)
{
	for (int i = 0; i < (int)Modules::Max; i++)
	{
		modules[i]->onPhase(p);
	}
}

/** @} */