// !!! GENERATED CODE
// !!! Do not change the below, unless you know what you are doing.
// !!!
// !!!

#ifndef GeneratedCodeAnimations_h
#define GeneratedCodeAnimations_h

#include <Arduino.h>
//#include "src/CommandStream.h"

namespace GeneratedCodeAnimations
{
	byte getAnimationCount();
	const uint16_t* getConfigValues(byte animationIndex);
	//CommandStream* GenerateSetupCommandStream();
	//CommandStream* GenerateCommandStreamByIndex(byte animationIndex);

	const char* const* getSetup();
	const char* getLoopByIndex(byte animationIndex);
	const char* const* getAnimationDataByIndex(byte animationIndex);
}

#endif // GeneratedCodeAnimations
