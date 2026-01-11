// DataSource.h

#ifndef _DataSource_h
#define _DataSource_h

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"

class DataSource
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
	virtual bool hasData() {}

	/**
	 * @brief Try to consume data from the source.
	 * @param out Pointer to the data.
	 * @return true if data is available, false otherwise.
	 */
	virtual bool tryConsumeData(char* out) {}

protected:
	bool validDataAvailable = false;
};

#endif

