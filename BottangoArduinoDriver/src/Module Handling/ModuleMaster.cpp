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
#include "../Outgoing.h"

void ModuleMaster::setupModules()
{
	static SerialSource serialSource;
	modules[(int)Modules::DataSource_1] = &serialSource;

#ifdef USE_SD_CARD_COMMAND_STREAM
	static SdCardSource sdCardSource;
	modules[(int)Modules::DataSource_2] = &sdCardSource;
#endif

	static AsciiCmdDecoder asciiDecoder;
	modules[(int)Modules::Decoder] = &asciiDecoder;
	asciiDecoder.setDataSource(&serialSource);

	static Parser parser;
	modules[(int)Modules::Parser] = &parser;
	parser.setCommandDecoder(&asciiDecoder);

	modules[(int)Modules::EffectorPool] = &BottangoCore::effectorPool;

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
		if (modules[i] != nullptr)
		{
			modules[i]->init();
		}
	}
}

void ModuleMaster::executePhase(Phase p)
{
	for (int i = 0; i < (int)Modules::Max; i++)
	{
		if (modules[i] != nullptr)
		{
			modules[i]->onPhase(p);
		}
	}
}

/*void* ModuleMaster::getModule(Modules moduleType)
{
	return modules[(int)moduleType];
}*/

/** @} */