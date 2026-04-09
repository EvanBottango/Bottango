// GENERATED CODE. DO NOT CHANGE
// Trigger animations automatically using the configuration in the app, or callbacks in the BottangoArduinoCallbacks.cpp file
// Look at the method "onLateLoop" in the callbacks file for examples

// Animation Index Key:
// 0: Default Animation


// !!! Do not change the below, unless you know what you are doing.
// !!!
// !!!


#include "GeneratedCodeAnimations.h"
#include "src/CodeCommandStreamDataSource.h"

namespace GeneratedCodeAnimations
{
	// setup
	const char SETUP_DATA_0[] PROGMEM = "rCtrl,B827EBD80000\nrSVPin,1,1000,2000,3000,1500\nrSVPin,2,1000,2000,3000,1500\nrSVPin,3,1000,2000,3000,1500\nsR,0,rSVPin,5,1000,2000,3000,1500\nsR,0,rSVPin,6,1000,2000,3000,1500\nsR,0,rSVPin,7,1000,2000,3000,1500\n";
	const char* const SETUP_DATAARRAY[] PROGMEM = { SETUP_DATA_0 };

	// animation 0, "Default Animation"
	const char ANIM_0_DATA0[] PROGMEM = "sSY,sC,1,0,5000,4096,1250,0,8192,-1250,0;2,0,5000,4096,1250,0,0,-1250,0;3,0,5000,4096,1250,0,8192,-1250,0;\nsR,0,sSY,sC,5,0,5000,4096,1250,0,0,-1250,0;6,0,5000,4096,1250,0,0,-1250,0;7,0,5000,4096,1250,0,8192,-1250,0;\nsSY,sC,1,5000,5000,8192,1250,0,4096,-1250,0;2,5000,5000,0,1250,0,4096,-1250,0;3,5000,5000,8192,1250,0,4096,-1250,0;\nsR,0,sSY,sC,5,5000,5000,0,1250,0,4096,-1250,0;6,5000,5000,0,1250,0,4096,-1250,0;7,5000,5000,8192,1250,0,4096,-1250,0;\n\n";
	const char* const ANIM_0_DATAARRAY[] PROGMEM = { ANIM_0_DATA0 };
	const char ANIM_0_LOOP[] PROGMEM = "";
	const uint16_t ANIM_0_CONFIG[] PROGMEM = { 0, 0, 0, 9, 0, 1, 0, 0 };


	// configs
	const uint16_t* const CONFIGS_ARRAY[] PROGMEM = { ANIM_0_CONFIG };

	CommandStream* GenerateSetupCommandStream()
	{
		return new CommandStream(new CodeCommandStreamDataSource(SETUP_DATAARRAY, 1));
	}

	CommandStream* GenerateCommandStreamByIndex(byte animationIndex)
	{
		switch (animationIndex)
		{
		case 0:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_0_DATAARRAY, 1, ANIM_0_LOOP));

		}
		return nullptr;
	}

	byte getAnimationCount()
	{
		return 1;
	}

	const uint16_t* getConfigValues(uint8_t animationIndex)
	{
		// AVR Needs to move it to sram    
#if defined(ARDUINO_ARCH_AVR)
#define CONFIG_SIZE 8

		static uint16_t copy[CONFIG_SIZE];
		const uint16_t* flashPtr =
			(const uint16_t*)pgm_read_word_near(&CONFIGS_ARRAY[animationIndex]);

		for (uint8_t i = 0; i < CONFIG_SIZE; i++)
		{
			copy[i] = pgm_read_word_near(&flashPtr[i]);
		}
		return copy;
#else
		// Everything else can use normal accessing
		return CONFIGS_ARRAY[animationIndex];
#endif
	}
}
