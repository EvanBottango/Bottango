// 
// 
// 
#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include "AnimationPlaybackControl.h"
#include "../BottangoCore.h"
#include "../PersistentConfigUtil.h"
#include "../DataSource/SdCardSource.h"
#include "../Module Handling/ModuleMaster.h"

void AnimationPlaybackControl::onPhase(Phase p)
{
	if (p != Phase::Logic)
	{
		return;
	}
}

void AnimationPlaybackControl::init()
{
#ifdef USE_SD_CARD_COMMAND_STREAM
	BottangoCore::mMaster.registerModuleInSecondaryDataSlot<SdCardSource>();
#elif USE_CODE_COMMAND_STREAM
	// ToDo for USE_CODE_COMMAND_STREAM
	// BottangoCore::mMaster.registerModuleInSecondaryDataSlot<ExportedCodeSource>();
#endif // USE_CODE_COMMAND_STREAM

#ifdef ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH
	if (PersistentConfigUtil::getUseExportedCommandStream())
	{
		if (BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary) != nullptr)
		{
			// Activate the secondary data source
			BottangoCore::mMaster.getModule<DataSource>(Modules::DataSource_Secondary)->setActiveSource(true);
		}
	}
#endif // ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH

}

void AnimationPlaybackControl::runSetup()
{

}

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)