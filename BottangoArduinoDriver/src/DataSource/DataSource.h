// DataSource.h

#ifndef _DataSource_h
#define _DataSource_h

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Module Handling/ModuleLoop.h"

class DataSource : public LoopModule
{
public:
	/**
	 * @brief Try to read data from the source.
	 */
	virtual void readData() {}

	/**
	 * @brief Returns true if there is data available to consume.
	 * @return true if data is available, false otherwise.
	 */
	virtual bool hasData()
	{
		return validDataAvailable;
	}

	/**
	 * @brief Try to consume data from the source.
	 * @param out Pointer to the data.
	 * @return true if data is available, false otherwise.
	 */
	virtual bool tryConsumeData(char** out)
	{
		return false;
	}

	virtual void resetBuffer() {}

	virtual bool isActiveSource()
	{
		return isActive;
	}

	virtual void setActiveSource(bool active)
	{
		isActive = active;
	}	

protected:
	bool validDataAvailable = false;
	bool isActive = false;
};

#endif