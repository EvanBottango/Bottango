// OfflineAnimationControl.h

#ifndef _OfflineAnimationControl_h
#define _OfflineAnimationControl_h

#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Module Handling/ModuleLoop.h"
#include "../CircularArray.h"
#include "../DataSource/StaticSecondaryDataSource.h"
#include "../Communication/Parser.h"
#include "AnimationConfiguration.h"



class AnimationPlaybackControl : public LoopModule
{
public:
	void onPhase(Phase p) override;
	void init() override;

	void stop();

	void updatePlaybackStatus();

	bool readyForNextCommand();

	bool complete();

private:
	CircularArray<AnimationConfiguration> _animationConfigs = CircularArray<AnimationConfiguration>(MAX_EXPORTED_ANIMATIONS);
	unsigned long _timeStartOfNextCommand = 0;
	unsigned long _timeEndOfLongestCommand = 0;
	int _currentPlayingIndex = -1;
	int _idleAnimIndex = -1;
	int _startingAnim = -1;
	bool _setupIsRunning = false;

	Parser* _parser = nullptr;
	OfflineDataSource* _offlineSource = nullptr;

	int getIndexOfAnimationToTrigger() const;

	void playAnimation(int index, bool loop);

#ifdef USE_SD_CARD_COMMAND_STREAM
	void loadConfig();
#endif // USE_SD_CARD_COMMAND_STREAM

#ifdef EXPORTED_ANIM_LOGGING
	void logConfig(AnimationConfiguration* config);
#endif // EXPORTED_ANIM_LOGGING
};

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
#endif // _OfflineAnimationControl_h