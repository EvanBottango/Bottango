// OfflineAnimationControl.h

#ifndef _OfflineAnimationControl_h
#define _OfflineAnimationControl_h

#include "../../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Module Handling/LoopModule.h"
#include "../CircularArray.h"
#include "../DataSource/StaticSecondaryDataSource.h"
#include "../Communication/Parser.h"
#include "AnimationConfiguration.h"
#include "../Modules/RelayComs/Relay.h"


class AnimationPlaybackControl : public LoopModule
{
public:
	void onPhase(Phase const p) override;
	void init() override;

	void stop(bool allowSetupStop);

	void updatePlaybackStatus();

	bool readyForNextCommand();

	bool complete() const;

	void setInvalidState();

private:
	CircularArray<AnimationConfiguration> _animationConfigs = CircularArray<AnimationConfiguration>(MAX_EXPORTED_ANIMATIONS);
	unsigned long _timeStartOfNextCommand = 0;
	unsigned long _timeEndOfLongestCommand = 0;
	int _currentPlayingIndex = -1;
	int _idleAnimIndex = -1;
	int _startingAnim = -1;
	bool _setupIsRunning = false;	
	bool _invalidState = false;

	Parser* _parser = nullptr;
	OfflineDataSource* _offlineSource = nullptr;

#ifdef RELAY_SUPPORTED
	Relay* _relay = nullptr;
	bool _registeredAllPeers = false;
#endif // RELAY_SUPPORTED

	int getIndexOfAnimationToTrigger() const;

	void playAnimation(uint8_t const index, bool const loop) const;

//#ifdef USE_SD_CARD_COMMAND_STREAM
	void loadConfig();
//#endif // USE_SD_CARD_COMMAND_STREAM

#ifdef EXPORTED_ANIM_LOGGING
	static void logConfig(const AnimationConfiguration* config);
#endif // EXPORTED_ANIM_LOGGING
};

#endif // (USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
#endif // _OfflineAnimationControl_h