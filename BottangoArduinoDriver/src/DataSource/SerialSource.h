// SerialSource.h

#ifndef _SerialSource_h
#define _SerialSource_h

#include <Arduino.h>
#include "CharStreamedSource.h"

/**
 * @brief Serial data source. It uses the default Arduino Serial interface to read incoming commands. Commands are expected to be terminated with a newline character ('\n') and include a hash for validation.
 */
class SerialSource : public CharStreamedSource
{
public:
	void onPhase(Phase p) override;
	void init() override;
	void readData() override;

private:

};

#endif