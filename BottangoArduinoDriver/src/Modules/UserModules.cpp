/**
 * @file UserModules.cpp
 * @brief Example implementation of user-defined custom modules.
 * @details This file demonstrates how to create and register custom modules without modifying core code.
 */

#include "../BottangoArduinoModules.h"

 // Only compile this file if user module setup is enabled
#ifdef USER_MODULE_SETUP_ENABLED

#include "../Module Handling/PhaseScheduler.h"
#include "../Module Handling/ModuleFactory.h"
#include "../Module Handling/LoopModule.h"

// === Example 1: Simple LED Blinker Module ===

class BlinkLEDModule : public LoopModule
{
public:
	void init() override
	{
		pinMode(LED_BUILTIN, OUTPUT);
		_lastToggleTime = millis();
	}

	void onPhase(Phase p) override
	{
		if (p == Phase::Update)
		{
			unsigned long currentTime = millis();
			if (currentTime - _lastToggleTime >= 1000) // Blink every second
			{
				_ledState = !_ledState;
				digitalWrite(LED_BUILTIN, _ledState ? HIGH : LOW);
				_lastToggleTime = currentTime;
			}
		}
	}

private:
	unsigned long _lastToggleTime = 0;
	bool _ledState = false;
};

// === Example 2: Module with Core Service Dependency ===

class CustomDataProcessorModule : public LoopModule
{
public:
	void setSerialSource(SerialSource* serial)
	{
		_serialSource = serial;
	}

	void init() override
	{
		// Validate that dependency was injected
		if (_serialSource == nullptr)
		{
			// Handle error (e.g., log or halt)
		}
	}

	void onPhase(Phase p) override
	{
		if (p == Phase::Update && _serialSource != nullptr)
		{
			// Do something with serial source
			// Example: Monitor for specific custom commands
		}
	}

private:
	SerialSource* _serialSource = nullptr;
};

// === Global Instances ===

static BlinkLEDModule g_blinkModule;
static CustomDataProcessorModule g_customProcessor;

// === User Setup Hook Implementation ===

void onUserModuleSetup(PhaseScheduler& scheduler, ModuleFactory& factory)
{
	// Register simple blink module with normal priority
	scheduler.addToLoop(&g_blinkModule, Priority::Normal);

	// Register custom processor with dependency injection
	g_customProcessor.setSerialSource(factory.get<SerialSource>());
	scheduler.addToLoop(&g_customProcessor, Priority::Late);

	// Note: init() is called automatically by addToLoop()
}

#endif // USER_MODULE_SETUP_ENABLED