#pragma once

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"
#include "../Services/ISchedulable.h"

class DataSource : public ISchedulable
{
public:
	/**
	 * @brief Returns true if there is data available to consume.
	 * @return true if data is available, false otherwise.
	 */
	virtual bool hasData()
	{
		return m_validDataAvailable;
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
	virtual void resetBuffer()
	{
	}

	/**
	 * @brief Helper, returns whether this data source is currently active
	 * @return True if this data source is active, false otherwise.
	 */
	virtual bool isActiveSource()
	{
		return m_isActive;
	}

	/**
	 * @brief Helper, to set whether this data source is currently active.
	 * @param active True to set this data source as active, false to set it as inactive.
	 */
	virtual void setActiveSource(bool active)
	{
		m_isActive = active;
	}	

protected:
	/**
	 * @brief Indicates, if valid data is available to consume
	 */
	bool m_validDataAvailable = false;

	/**
	 * @brief Indicates, if this source is active or not
	 */
	bool m_isActive = false;
};