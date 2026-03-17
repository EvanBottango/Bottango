#pragma once

#include <Arduino.h>
#include "../Modules/AnimationConfiguration.h"
#include "../DataSource/DataSource.h"

class StaticSecondaryDataSource : public DataSource
{
public:
	virtual bool openSetup()
	{
		return false;
	}

	virtual bool openAnimation(uint8_t animIndex, bool loop)
	{
		return false;
	}

	virtual void prepareNextCommand() {};

	virtual void peekNextCommand(char* out) {};

	virtual bool getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config) const
	{
		return false;
	}

	virtual bool dataComplete() const
	{
		return _dataComplete;
	};

protected:
	bool _dataComplete = false;
};