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
		return _validDataAvailable;
	}

	/**
	 * @brief Try to consume data from the source.
	 * @param out Pointer to a char* variable that will be set to point to the data if available
	 * @return true if data is available, false otherwise.
	 */
	virtual bool tryConsumeData(char** out)
	{
		return false;
	}

	/**
	 * @brief Reset the data buffer.
	 */
	virtual void resetBuffer() {}

	/**
	 * @brief Helper, returns whether this data source is currently active
	 * @return True if this data source is active, false otherwise.
	 */
	virtual bool isActiveSource()
	{
		return _isActive;
	}

	/**
	 * @brief Helper, to set whether this data source is currently active.
	 * @param active True to set this data source as active, false to set it as inactive.
	 */
	virtual void setActiveSource(bool active)
	{
		_isActive = active;
	}	

protected:
	/**
	 * @brief Indicates, if valid data is available to consume
	 */
	bool _validDataAvailable = false;

	/**
	 * @brief Indicates, if this source is active or not
	 */
	bool _isActive = false;
};

#endif