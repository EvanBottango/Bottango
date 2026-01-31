// 
// 
// 

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include "OfflineAnimationControl.h"

void OfflineAnimationControl::onPhase(Phase p)
{
	if (p != Phase::Logic)
	{
		return;
	}
}

void OfflineAnimationControl::runSetup()
{
	// At the moment, we have to assume that the setup file is open, directly after boot

}

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)