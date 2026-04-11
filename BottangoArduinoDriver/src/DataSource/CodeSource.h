#pragma once
#include "../../BottangoArduinoModules.h"
#ifdef USE_CODE_COMMAND_STREAM
#include "StaticSecondaryDataSource.h"
#include "../Modules/AnimationPlaybackControl.h"

class CodeSource : public OfflineDataSource
{
public:
	void onPhase(Phase p) override;
	void init() override;
	bool openSetup() override;
	bool openAnimation(uint8_t animIndex, bool loop) override;
	void prepareNextCommand() override;
	bool peekNextCommand(char* out) override;
	bool tryConsumeData(char** out) override;
	void resetBuffer() override;
	bool getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const override;

private:
	// ==== Buffer and file management ====
	char _commandBuffer[MAX_COMMAND_LENGTH] = {};
	const char* const* _dataArray = nullptr;
	int8_t _arrayLength = 0;
	const char* _loopCharStream = nullptr;
	int8_t _dataArrayIndex = 0;
	unsigned int _currentDataStringLength = 0;
	unsigned int _travel = 0;
	bool _shouldLoop = false;

	bool getNextCommand(char* buffer, bool peek = false);
	void parseConfiguration(AnimationConfiguration* config, const uint16_t* parsedValue) const;
	void incrementArrayIndex();
};

#endif // USE_CODE_COMMAND_STREAM