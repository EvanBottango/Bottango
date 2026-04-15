/**
 * @defgroup PhaseScheduler Phase-based execution scheduler
 * @{
 */

#include "PhaseScheduler.h"
#include "ModuleFactory.h"
#include "../BottangoCore.h"
#include "../../BottangoArduinoModules.h"

#include "../DataSource/SerialSource.h"
#include "../Communication/AsciiCmdDecoder.h"
#include "../Communication/Parser.h"
#include "../Modules/AnimationPlaybackControl.h"
#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLightsModule.h"
#include "../Modules/Audio/I2SAudioModule.h"

// Forward declaration of global factory (defined in BottangoCore)
extern ModuleFactory g_moduleFactory;

void PhaseScheduler::buildModules()
{
	// add modules in STRICT order.

	// will init in the order shown below after they are all added to the list
	// During each loop phase, each module will respond to phase in the order added to the internal list here.

	// note that no creation of the moduels, configuraiton, etc. is handled here. It leaves that job up to
	// module factory.

	// This just grabs what it wants in the right order

	// numbers in comments are if every module is enabled. We don't ever get by index, so just for readability
	// here in comments.

	static LoopModule *const orderedModules[] = {
		// 1 - Core - Serial Source
		g_moduleFactory.get<SerialSource>(),

		// 2 - Optional - Offline Playback Control
		// side note - in your latest, you had this after effector pool
		// in earlier modulemaster version, you had this as item 2 in the list (which I recreated here)
		// eventually this will need to live in the right real slot
		// all of this is just an example for now
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
		g_moduleFactory.get<AnimationPlaybackControl>(),
#endif

		// 3 - Optional - Relay
#ifdef RELAY_SUPPORTED
		g_moduleFactory.get<Relay>(),
#endif

		// 4 - Core - Command Decoder
		g_moduleFactory.get<AsciiCmdDecoder>(),

		// 5 - Core - Command Parser
		g_moduleFactory.get<Parser>(),

		// 6 - Core - EffectorPool
		// Heads up! This is an example of adding something to the scheduler
		// without it being a module owned by module factory
		// because it will never change, it doesn't need to be dynamic and owned by module factory.
		// BottangoCore just makes the object, and phase scheduler hard codes it into it's own list by adding it at the right time.
		// effectorPool is loop valid just by implementing LoopModule
		&BottangoCore::effectorPool,

		// 7 - Optional - Stop Button
#ifdef STOP_BUTTON_SUPPORTED
		g_moduleFactory.get<StopButtonModule>(),
#endif

		// 8 - Optional - Status Lights
#ifdef ENABLE_STATUS_LIGHTS
		g_moduleFactory.get<StatusLightsModule>(),
#endif

		// 9 - Optional - I2S Audio
#ifdef AUDIO_SD_I2S
		g_moduleFactory.get<I2SAudioModule>(),
#endif

		// 10 - example someday we want to add a button debouncer
		// anything can own it, and we can put it anywhere in the list hardcoded
		// all you need to do is make it implement LoopModule and add it here.
		// the compiler will size the table from the entries.
#ifdef BUTTON_DEBOUNCING
		&thingThatOwnsButtonDebouncer.ButtonDebouncer,
#endif
	};

	_orderedModules = orderedModules;
	moduleCount = sizeof(orderedModules) / sizeof(orderedModules[0]);
}

void PhaseScheduler::initModules() const
{
	// Initialize all modules in execution order
	for (uint8_t i = 0; i < moduleCount; i++)
	{
		if (_orderedModules[i] != nullptr)
		{
			_orderedModules[i]->init();
		}
	}
}

void PhaseScheduler::executePhase(Phase p) const
{
	// Execute all modules in priority order
	for (uint8_t i = 0; i < moduleCount; i++)
	{
		if (_orderedModules[i] != nullptr)
		{
			_orderedModules[i]->onPhase(p);
		}
	}
}

/** @} */
