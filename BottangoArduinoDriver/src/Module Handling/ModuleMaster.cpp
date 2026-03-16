/**
 * @defgroup ModuleSystem Module master system
 * @{
 */

#include "ModuleMaster.h"
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
#include "../Outgoing.h"

void ModuleMaster::setupModules()
{
	// The serial data source is always present and active
	SerialSource* serialSource = registerModule<SerialSource>(Modules::DataSource_Serial);
	serialSource->setActiveSource(true);


#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	registerModule<AnimationPlaybackControl>(Modules::AnimPlaybackCntrl);
#endif

	AsciiCmdDecoder* asciiDecoder = registerModule<AsciiCmdDecoder>(Modules::Decoder);
	asciiDecoder->setDataSource(serialSource);

	Parser* parser = registerModule<Parser>(Modules::Parser);
	parser->setCommandDecoder(asciiDecoder);

	_modules[(int)Modules::EffectorPool] = &BottangoCore::effectorPool;

#ifdef STOP_BUTTON_SUPPORTED
	registerModule<StopButtonModule>(Modules::StopButton);
#endif // STOP_BUTTON_SUPPORTED

#ifdef ENABLE_STATUS_LIGHTS
	registerModule<StatusLightsModule>(Modules::StatusLights);
#endif // ENABLE_STATUS_LIGHTS

#ifdef AUDIO_SD_I2S
	static I2SAudioModule audioModule;
	_modules[(int)Modules::AudioI2S] = &audioModule;
	InterfaceRegistry::registerInterface(Modules::AudioI2S, static_cast<IAudioPlayback*>(&audioModule));
#endif
}

void ModuleMaster::initModules()
{
	for (int i = 0; i < (int)Modules::Max; i++)
	{
		if (_modules[i] != nullptr)
		{
			_modules[i]->init();
		}
	}
}

void ModuleMaster::executePhase(Phase p)
{
	for (int i = 0; i < (int)Modules::Max; i++)
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