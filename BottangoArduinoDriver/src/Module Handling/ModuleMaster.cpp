/**
 * @defgroup ModuleSystem Module master system
 * @{
 */

#include "ModuleMaster.h"
#include "ModuleFactory.h"
#include "../BottangoCore.h"
#include "../../BottangoArduinoModules.h"

#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"

#include "../Communication/AsciiCmdDecoder.h"
#include "../Communication/Parser.h"

#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLightsModule.h"
#include "../Modules/Audio/I2SAudioModule.h"
#include "../Modules/AnimationPlaybackControl.h"
#include "../Modules/RelayComs/RelayESPNow.h"
#include "../Modules/OutgoingSerial.h"

#ifdef RELAY_COMS_ESPNOW
#include "../Modules/OutgoingRelay.h"
#endif

 // Global Factory instance
static ModuleFactory g_moduleFactory;

void ModuleMaster::setupModules()
{
	// Delegate to factory
	g_moduleFactory.setup();
	g_moduleFactory.wireModules();

	// Register modules in the local array (for backward compatibility)
	// This allows existing code using getModule() to continue working
	_modules[static_cast<int>(Modules::DataSource_Serial)] = g_moduleFactory.get<SerialSource>();
	_modules[static_cast<int>(Modules::Decoder)] = g_moduleFactory.get<AsciiCmdDecoder>();
	_modules[static_cast<int>(Modules::Parser)] = g_moduleFactory.get<Parser>();

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	_modules[static_cast<int>(Modules::AnimPlaybackCntrl)] = g_moduleFactory.get<AnimationPlaybackControl>();
#endif

#ifdef RELAY_SUPPORTED
	_modules[static_cast<int>(Modules::RelayComs)] = g_moduleFactory.get<RelayESPNow>();
#endif

#ifdef STOP_BUTTON_SUPPORTED
	_modules[static_cast<int>(Modules::StopButton)] = g_moduleFactory.get<StopButtonModule>();
#endif

#ifdef ENABLE_STATUS_LIGHTS
	_modules[static_cast<int>(Modules::StatusLights)] = g_moduleFactory.get<StatusLightsModule>();
#endif

#ifdef AUDIO_SD_I2S
	_modules[static_cast<int>(Modules::AudioI2S)] = g_moduleFactory.get<I2SAudioModule>();
#endif

	// ==== Effector Pool (still managed by BottangoCore) ====
	_modules[static_cast<int>(Modules::EffectorPool)] = &BottangoCore::effectorPool;
}

void ModuleMaster::initModules() const
{
	for (int i = 0; i < static_cast<int>(Modules::Max); i++)
	{
		if (_modules[i] != nullptr)
		{
			_modules[i]->init();
		}
	}
}

void ModuleMaster::executePhase(Phase const p) const
{
	for (int i = 0; i < static_cast<int>(Modules::Max); i++)
	{
		if (_modules[i] != nullptr)
		{
			_modules[i]->onPhase(p);
		}
	}
}

/*void* ModuleMaster::getModule(Modules moduleType)
{
	return modules[(int)moduleType];
}*/

/** @} */