#include "ScheduleManager.h"
#include "ServiceFactory.h"
#include "../System/BottangoCore.h"
#include "../../BottangoArduinoModules.h"

/*#include "../DataSource/SerialSource.h"
#include "../Communication/AsciiCmdDecoder.h"
#include "../Communication/Parser.h"
#include "../Modules/AnimationPlaybackControl.h"
#include "../DataSource/EspNowSource.h"
#include "../DataSource/RS485Source.h"
#include "../Modules/RelayComs/Relay.h"
#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLights.h"
#include "../Modules/Audio/I2SAudioModule.h"*/

void ScheduleManager::buildServices()
{
	// Add services in STRICT order.
	//
	// Will init in the order shown below after they are all added to the list
	// During each loop phase, each service will respond to phase in the order added to the internal list here.
	//
	// Note that no creation of the services, configuration, etc. is handled here. It leaves that job up to
	// service factory.
	// This just grabs what it wants in the right order

	/*static ISchedulable* const orderedServices[] =
	{
		ServiceFactory::get<SerialSource>(),				// [Core] - Serial Source

#ifdef RELAY_SUPPORTED
		ServiceFactory::get<Relay>(),						// [Optional] - Relay Communications
#ifdef RELAY_COMS_ESPNOW		
		ServiceFactory::get<EspNowSource>(),				// [Optional] - ESP Now Source
#endif
#ifdef RELAY_COMS_RS485
		ServiceFactory::get<RS485Source>(),					// [Optional] - RS485 Source
#endif
#endif		

		ServiceFactory::get<AsciiCmdDecoder>(),				// [Core] - Command Decoder		
		ServiceFactory::get<Parser>(),						// [Core] - Command Parser

#ifdef STOP_BUTTON_SUPPORTED
		ServiceFactory::get<StopButtonModule>(),			// [Optional] - Stop Button
#endif

#ifdef ENABLE_STATUS_LIGHTS
		ServiceFactory::get<StatusLights>(),				// [Optional] - Status Lights
#endif

#ifdef AUDIO_SD_I2S
		ServiceFactory::get<I2SAudioModule>(),				// [Optional] - I2S Audio
#endif

#ifdef OFFLINE_DATA_SOURCE_ENABLED
		ServiceFactory::get<AnimationPlaybackControl>(),	// [Optional] - Offline Playback Control
#endif

		&BottangoCore::effectorPool,						// [Core] - EffectorPool. Needs to be last, as they might depend on other modules being initialized first
	};

	m_orderedServices = orderedServices;
	m_servicesCount = sizeof(orderedServices) / sizeof(orderedServices[0]);*/
}

void ScheduleManager::initServices() const
{
	// Initialize all services in execution order
	for (uint8_t i = 0; i < m_servicesCount; i++)
	{
		if (m_orderedServices[i] != nullptr)
		{
			m_orderedServices[i]->init();
		}
	}
}

void ScheduleManager::executePhase(Phase const p) const
{
	// Execute all services in priority order
	for (uint8_t i = 0; i < m_servicesCount; i++)
	{
		if (m_orderedServices[i] != nullptr)
		{
			m_orderedServices[i]->onPhase(p);
		}
	}
}