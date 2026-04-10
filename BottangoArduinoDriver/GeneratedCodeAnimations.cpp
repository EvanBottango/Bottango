// GENERATED CODE. DO NOT CHANGE
// Trigger animations automatically using the configuration in the app, or callbacks in the BottangoArduinoCallbacks.cpp file
// Look at the method "onLateLoop" in the callbacks file for examples

// Animation Index Key:
// 0: TestAnimation
// 1: TestAnimation 02
// 2: TestAnimationLoop
// 3: TestAnimationIdle
// 4: TestAnimationStartupLoop


// !!! Do not change the below, unless you know what you are doing.
// !!!
// !!!


#include "GeneratedCodeAnimations.h"
//#include "src/CodeCommandStreamDataSource.h"

namespace GeneratedCodeAnimations
{
	// setup
	const char SETUP_DATA_0[] PROGMEM = "rSVPin,0,1000,2000,3000,1500\nrSVPin,1,1000,2000,3000,1500\nrSVPin,2,1000,2000,3000,1500\n";
	const char* const SETUP_DATAARRAY[] PROGMEM = { SETUP_DATA_0 };

	// animation 0, "TestAnimation1"
	const char ANIM_0_DATA0[] PROGMEM = "sC,0,0,2000,4096,513,0,7110,-763,814\nsC,2,0,2000,4096,513,0,1413,-763,-814\nsC,1,0,2000,4096,513,0,2332,-763,-814\nsC,0,2000,1000,7110,331,-354,2277,-331,931\nsC,2,2000,1000,1413,331,354,6322,-331,-889\nsC,1,2000,1000,2332,331,354,7626,-331,-772\nsC,0,3000,1000,2277,347,-976,1745,-314,320\nsC,2,3000,1000,6322,347,932,6748,-314,-237\nsC,1,3000,1000,7626,347,809,2016,-314,33\nsC,1,4000,1000,2016,377,-40,8192,-280,0\nsC,0,4000,1000,1745,377,-383,0,-280,0\nsC,2,4000,1000,6748,377,283,8192,-280,0\nsC,1,5000,1000,8192,281,0,6260,-377,314\nsC,0,5000,1000,0,281,0,2042,-377,-314\nsC,2,5000,1000,8192,281,0,5378,-377,313\nsC,1,6000,1000,6260,313,-261,4147,-347,244\nsC,0,6000,1000,2042,313,261,5572,-347,-234\nsC,2,6000,1000,5378,313,-260,1525,-347,165\nsC,2,7000,1000,1525,331,-157,3073,-331,-319\nsC,1,7000,1000,4147,331,-232,2694,-331,21\nsC,0,7000,1000,5572,331,223,4029,-331,174\nsC,0,8000,2000,4029,762,-402,4096,-513,0\nsC,1,8000,2000,2694,762,-49,4096,-513,0\nsC,2,8000,2000,3073,762,734,4096,-513,0\n\n";
	const char* const ANIM_0_DATAARRAY[] PROGMEM = { ANIM_0_DATA0 };
	const char ANIM_0_LOOP[] PROGMEM = "";
	const uint16_t ANIM_0_CONFIG[] PROGMEM = { 0, 0, 0, 10, 0, 1, 0, 0 };


	// configs
	const uint16_t* const CONFIGS_ARRAY[] PROGMEM = { ANIM_0_CONFIG };

	/*CommandStream* GenerateSetupCommandStream()
	{
		return new CommandStream(new CodeCommandStreamDataSource(SETUP_DATAARRAY, 1));
	}

	CommandStream* GenerateCommandStreamByIndex(byte animationIndex)
	{
		switch (animationIndex)
		{
		case 0:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_0_DATAARRAY, 1, ANIM_0_LOOP));
		case 1:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_1_DATAARRAY, 1, ANIM_1_LOOP));
		case 2:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_2_DATAARRAY, 1, ANIM_2_LOOP));
		case 3:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_3_DATAARRAY, 1, ANIM_3_LOOP));
		case 4:
			return new CommandStream(new CodeCommandStreamDataSource(ANIM_4_DATAARRAY, 1, ANIM_4_LOOP));

		}
		return nullptr;
	}*/

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

	const char* const* getSetup()
	{
		return SETUP_DATAARRAY;
	}

	const char* getLoopByIndex(byte animationIndex)
	{
		switch (animationIndex)
		{
		case 0:
			return ANIM_0_LOOP;
		}
		return nullptr;
	}

	const char* const* getAnimationDataByIndex(byte animationIndex)
	{
		switch (animationIndex)
		{
		case 0:
			return ANIM_0_DATAARRAY;
		}
		return nullptr;
	}
}