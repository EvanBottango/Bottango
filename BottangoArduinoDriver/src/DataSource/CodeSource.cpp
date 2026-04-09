#include "CodeSource.h"

#ifdef USE_CODE_COMMAND_STREAM
#include "../GeneratedCodeAnimations.h"

void CodeSource::onPhase(Phase p)
{
	// Only read data during the Communication phase
	if (p != Phase::Communication)
	{
		return;
	}
}

void CodeSource::init()
{
	_commandBuffer[0] = '\0';
}

bool CodeSource::openSetup()
{

}

bool CodeSource::getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const
{
	if (animIndex >= GeneratedCodeAnimations::getAnimationCount())
	{
		return false;
	}

	const uint16_t* parsedValue = GeneratedCodeAnimations::getConfigValues(animIndex);
	parseConfiguration(config, parsedValue);
	
	return true;
}

void CodeSource::parseConfiguration(AnimationConfiguration* config, const uint16_t* parsedValue) const
{
	if (parsedValue[0] > 0)
	{
		config->flags |= ANIM_PLAY_ON_START_FLAG;
	}

	if (parsedValue[1] > 0)
	{
		config->flags |= ANIM_LOOP_ON_START_FLAG;
	}

	if (parsedValue[2] > 0)
	{
		config->flags |= ANIM_IDLE_FLAG;
	}

	config->playOnPin = parsedValue[3];

	if (parsedValue[4] > 0)
	{
		config->flags |= ANIM_LOOPING_FLAG;
	}

	if (parsedValue[5] == 0)
	{
		config->flags |= ANIM_PLAY_ON_PIN_LOW_FLAG;
	}
	else if (parsedValue[5] == 1)
	{
		config->flags |= ANIM_PLAY_ON_PIN_HIGH_FLAG;
	}
	else if (parsedValue[5] == 2)
	{
		config->flags |= ANIM_PLAY_ON_PIN_ANALOG_FLAG;
	}

	config->buttonLadderMin = parsedValue[6];
	config->buttonLadderMax = parsedValue[7];
}

#endif // USE_CODE_COMMAND_STREAM