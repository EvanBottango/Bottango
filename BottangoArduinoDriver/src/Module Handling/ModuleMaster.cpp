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
#include "../Modules/RelayComs/RelayESPNow.h"
#include "../Modules/OutgoingSerial.h"

#ifdef RELAY_COMS_ESPNOW
#include "../Modules/OutgoingRelay.h"
#endif

void ModuleMaster::setupModules()
{
	// ==== Serial data source ====
	// The serial data source is always present and active
	SerialSource* serialSource = registerModule<SerialSource>(Modules::DataSource_Serial);
	serialSource->setActiveSource(true);

	// ==== Offline Playback Control ====
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	registerModule<AnimationPlaybackControl>(Modules::AnimPlaybackCntrl);
#endif

	// ==== Relay Modules ====
#if defined(RELAY_SUPPORTED)
	registerModule<RelayESPNow>(Modules::RelayComs);

	// Relay Output
	static OutgoingRelayImpl outgoingRelay;
	OutgoingRelay::bind(&outgoingRelay);
	outgoingRelay.setRelayComs(getModule<Relay>(Modules::RelayComs));
#endif

	// ==== Command Decoder ====
	AsciiCmdDecoder* asciiDecoder = registerModule<AsciiCmdDecoder>(Modules::Decoder);
	asciiDecoder->setDataSource(serialSource);

	// ==== Command Parser ====
	Parser* parser = registerModule<Parser>(Modules::Parser);
	parser->setCommandDecoder(asciiDecoder);

	// ==== Effector Pool ====
	_modules[(int)Modules::EffectorPool] = &BottangoCore::effectorPool;

	// ===== Optional Modules ====
#ifdef STOP_BUTTON_SUPPORTED
	registerModule<StopButtonModule>(Modules::StopButton);
#endif // STOP_BUTTON_SUPPORTED

#ifdef ENABLE_STATUS_LIGHTS
	registerModule<StatusLightsModule>(Modules::StatusLights);
#endif // ENABLE_STATUS_LIGHTS

#ifdef AUDIO_SD_I2S
	registerModule<I2SAudioModule>(Modules::AudioI2S);
#endif

	// ==== Setup Output bindings ====
	// Serial Output
	static OutgoingSerialImpl outgoingSerialImpl;
	OutgoingSerial::bind(&outgoingSerialImpl);

	// Host Output (default to Serial)
	Outgoing::bind(&outgoingSerialImpl);
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