#include "ServiceFactory.h"
#include "../System/BottangoCore.h"
#include "../../BottangoArduinoModules.h"

// ToDo: Turned off during this step of staged refactor
/*#include "../DataSource/SerialSource.h"
#include "../DataSource/SdCardSource.h"*/

// ToDo: Turned off during this step of staged refactor
/*#include "../Communication/AsciiCmdDecoder.h"
#include "../Communication/Parser.h"*/

// ToDo: Turned off during this step of staged refactor
/*#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLights.h"
#include "../Modules/Audio/I2SAudioModule.h"
#include "../Modules/AnimationPlaybackControl.h"
#include "../Modules/RelayComs/RelayESPNow.h"
#include "../Modules/RelayComs/RelayRS485.h"
#include "../Modules/OutgoingSerial.h"*/

#ifdef RELAY_SUPPORTED
// ToDo: Turned off during this step of staged refactor
//#include "../Modules/OutgoingRelay.h"
#endif

void ServiceFactory::setup()
{
	getInstance().setup_Impl();
}

void ServiceFactory::wireServices()
{
	getInstance().wireModules_Impl();
}

template <typename T>
T* ServiceFactory::createModule()
{
	static T moduleInstance;
	return &moduleInstance;
}

void ServiceFactory::setup_Impl()
{
	// ==== Create Core Modules ====
	// ToDo: Turned off during this step of staged refactor
	//m_serialSource = createModule<SerialSource>();
	//m_asciiDecoder = createModule<AsciiCmdDecoder>();
	//m_parser = createModule<Parser>();

#ifdef OFFLINE_DATA_SOURCE_ENABLED
	// ToDo: Turned off during this step of staged refactor
	//m_animPlaybackControl = createModule<AnimationPlaybackControl>();
#endif // OFFLINE_DATA_SOURCE_ENABLED

#ifdef RELAY_SUPPORTED
#ifdef RELAY_COMS_ESPNOW
	// ToDo: Turned off during this step of staged refactor
	//m_relayComs = createModule<RelayESPNow>();
	//m_espNowSource = createModule<EspNowSource>();
#endif
#ifdef RELAY_COMS_RS485
	// ToDo: Turned off during this step of staged refactor
	//m_relayComs = createModule<RelayRS485>();
	//m_rs485Source = createModule<RS485Source>();
#endif
#endif


	// ==== Create Optional Modules ====
#ifdef STOP_BUTTON_SUPPORTED
	// ToDo: Turned off during this step of staged refactor
	//m_stopButton = createModule<StopButtonModule>();
#endif

#ifdef ENABLE_STATUS_LIGHTS
	// ToDo: Turned off during this step of staged refactor
	//m_statusLights = createModule<StatusLights>();
#endif

#ifdef AUDIO_SD_I2S
	// ToDo: Turned off during this step of staged refactor
	//m_audioI2S = createModule<I2SAudioModule>();
#endif
}

void ServiceFactory::wireModules_Impl() const
{
	// ToDo: Turned off during this step of staged refactor
	// ==== Wire Serial Source ====
	/*m_serialSource->setActiveSource(true);

	// ==== Wire Decoder ====
	m_asciiDecoder->setDataSource(m_serialSource);

	// ==== Wire Parser ====
	m_parser->setCommandDecoder(m_asciiDecoder);

#ifdef RELAY_SUPPORTED
	// ==== Wire Relay Coms ====
	static OutgoingRelayImpl outgoingRelay;
	OutgoingRelay::bind(&outgoingRelay);
	outgoingRelay.setRelayComs(m_relayComs);
#endif

	// ==== Setup Output bindings ====
	static OutgoingSerialImpl outgoingSerialImpl;
	OutgoingSerial::bind(&outgoingSerialImpl);
	Outgoing::bind(&outgoingSerialImpl);*/
}