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
#include "../Modules/RelayComs/RelayESPNow.h"
#include "../Modules/StopButtonModule.h"
#include "../Modules/StatusLightsModule.h"
#include "../Modules/Audio/I2SAudioModule.h"

 // Forward declaration of global factory (defined in BottangoCore)
extern ModuleFactory g_moduleFactory;

void PhaseScheduler::registerCoreModule(LoopModule* module, Priority priority)
{
	if (_coreModuleCount < MAX_CORE_MODULES && module != nullptr)
	{
		_coreModules[_coreModuleCount].module = module;
		_coreModules[_coreModuleCount].priority = priority;
		_coreModuleCount++;
	}
	// TODO: Error handling if array is full
}

void PhaseScheduler::setupCorePhases()
{
	// ==== Register Core Modules with Priorities ====

	// VeryEarly: Data Sources (need to run first to receive data)
	registerCoreModule(g_moduleFactory.get<SerialSource>(), Priority::VeryEarly);

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	// Offline data source (if present in slot)
	LoopModule* offlineSource = g_moduleFactory.getOfflineDataSource();
	if (offlineSource != nullptr)
	{
		registerCoreModule(offlineSource, Priority::VeryEarly);
	}
#endif

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	// Secondary data source (if present in slot)
	LoopModule* secondarySource = g_moduleFactory.getSecondaryDataSource();
	if (secondarySource != nullptr)
	{
		registerCoreModule(secondarySource, Priority::VeryEarly);
	}

	// Relay communication module
	registerCoreModule(g_moduleFactory.get<RelayESPNow>(), Priority::VeryEarly);
#endif

	// Early: Command processing pipeline
	registerCoreModule(g_moduleFactory.get<AsciiCmdDecoder>(), Priority::Early);
	registerCoreModule(g_moduleFactory.get<Parser>(), Priority::Early);

	// Normal: Effector Pool (from BottangoCore)
	registerCoreModule(&BottangoCore::effectorPool, Priority::Normal);

	// Late: Optional modules and playback control
#ifdef STOP_BUTTON_SUPPORTED
	registerCoreModule(g_moduleFactory.get<StopButtonModule>(), Priority::Late);
#endif

#ifdef ENABLE_STATUS_LIGHTS
	registerCoreModule(g_moduleFactory.get<StatusLightsModule>(), Priority::Late);
#endif

#ifdef AUDIO_SD_I2S
	registerCoreModule(g_moduleFactory.get<I2SAudioModule>(), Priority::Late);
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	// Animation playback must run after effector pool
	registerCoreModule(g_moduleFactory.get<AnimationPlaybackControl>(), Priority::Late);
#endif

	// Sort all core modules by priority
	sortModulesByPriority();
}

void PhaseScheduler::addToLoop(LoopModule* userModule, Priority priority)
{
	if (_userModuleCount >= MAX_USER_MODULES)
	{
		// TODO: Error handling (array full)
		return;
	}

	if (userModule == nullptr)
	{
		// TODO: Error handling (null pointer)
		return;
	}

	// Add to user module array
	_userModules[_userModuleCount].module = userModule;
	_userModules[_userModuleCount].priority = priority;
	_userModuleCount++;

	// Initialize immediately
	userModule->init();

	// Re-sort execution order to include new module
	sortModulesByPriority();
}

void PhaseScheduler::sortModulesByPriority()
{
	// Combine core and user modules into execution order array
	_executionCount = 0;

	// Simple insertion sort (suitable for small arrays)
	// We build a sorted array by merging core and user modules

	uint8_t coreIdx = 0;
	uint8_t userIdx = 0;

	while (coreIdx < _coreModuleCount || userIdx < _userModuleCount)
	{
		// Determine which module to add next (lowest priority first)
		bool takeCore = false;

		if (coreIdx >= _coreModuleCount)
		{
			takeCore = false; // No more core modules, take user
		}
		else if (userIdx >= _userModuleCount)
		{
			takeCore = true; // No more user modules, take core
		}
		else
		{
			// Both available, compare priorities
			takeCore = (_coreModules[coreIdx].priority <= _userModules[userIdx].priority);
		}

		if (takeCore)
		{
			_executionOrder[_executionCount++] = _coreModules[coreIdx].module;
			coreIdx++;
		}
		else
		{
			_executionOrder[_executionCount++] = _userModules[userIdx].module;
			userIdx++;
		}
	}
}

void PhaseScheduler::initModules() const
{
	// Initialize all modules in execution order
	for (uint8_t i = 0; i < _executionCount; i++)
	{
		if (_executionOrder[i] != nullptr)
		{
			_executionOrder[i]->init();
		}
	}
}

void PhaseScheduler::executePhase(Phase p) const
{
	// Execute all modules in priority order
	for (uint8_t i = 0; i < _executionCount; i++)
	{
		if (_executionOrder[i] != nullptr)
		{
			_executionOrder[i]->onPhase(p);
		}
	}
}

/** @} */