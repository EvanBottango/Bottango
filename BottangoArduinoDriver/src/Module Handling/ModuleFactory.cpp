/**
 * @defgroup ModuleFactory Module factory system
 * @{
 */

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

template <typename T>
T* ModuleFactory::createModule()
{
	static T moduleInstance;
	return &moduleInstance;
}

void ModuleFactory::setup()
{
	// ==== Create Core Modules ====
	_serialSource = createModule<SerialSource>();
	_asciiDecoder = createModule<AsciiCmdDecoder>();
	_parser = createModule<Parser>();

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	_animPlaybackControl = createModule<AnimationPlaybackControl>();
#endif

#ifdef RELAY_SUPPORTED
	_relayComs = createModule<RelayESPNow>();
#endif

	// ==== Create Optional Modules ====
#ifdef STOP_BUTTON_SUPPORTED
	_stopButton = createModule<StopButtonModule>();
#endif

#ifdef ENABLE_STATUS_LIGHTS
	_statusLights = createModule<StatusLightsModule>();
#endif

#ifdef AUDIO_SD_I2S
	_audioI2S = createModule<I2SAudioModule>();
#endif
}

void ModuleFactory::wireModules()
{
	// ==== Wire Serial Source ====
	_serialSource->setActiveSource(true);

	// ==== Wire Decoder ====
	_asciiDecoder->setDataSource(_serialSource);

	// ==== Wire Parser ====
	_parser->setCommandDecoder(_asciiDecoder);

#ifdef RELAY_SUPPORTED
	// ==== Wire Relay Coms ====
	static OutgoingRelayImpl outgoingRelay;
	OutgoingRelay::bind(&outgoingRelay);
	outgoingRelay.setRelayComs(_relayComs);
#endif

	// ==== Setup Output bindings ====
	static OutgoingSerialImpl outgoingSerialImpl;
	OutgoingSerial::bind(&outgoingSerialImpl);
	Outgoing::bind(&outgoingSerialImpl);
}

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
template <typename T>
T* ModuleFactory::placeInOfflineSlot()
{
	T* moduleInstance = _slotOfflineDataSource.place<T>();

	// Wire dependencies if needed (can be extended later)
	// ...

	// Initialize immediately
	moduleInstance->init();

	return moduleInstance;
}

LoopModule* ModuleFactory::getOfflineDataSource()
{
	return _slotOfflineDataSource.get();
}
#endif // USE_SD_CARD_COMMAND_STREAM || USE_CODE_COMMAND_STREAM

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
template <typename T>
T* ModuleFactory::placeInSecondarySlot()
{
	T* moduleInstance = _slotSecondaryDataSource.place<T>();

	// Wire dependencies if needed (can be extended later)
	// ...

	// Initialize immediately
	moduleInstance->init();

	return moduleInstance;
}

LoopModule* ModuleFactory::getSecondaryDataSource()
{
	return _slotSecondaryDataSource.get();
}
#endif // RELAY_SUPPORTED || USE_ESP32_WIFI

/** @} */