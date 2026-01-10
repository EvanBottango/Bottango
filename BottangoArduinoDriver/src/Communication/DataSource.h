// DataSource.h

#ifndef _DataSource_h
#define _DataSource_h

#include <Arduino.h>
#include "../../BottangoArduinoConfig.h"

class DataSource
{
public:
	virtual void readData() {}

	virtual bool hasData() {}

	virtual bool tryConsumeData(char* out) {}
};

#endif

