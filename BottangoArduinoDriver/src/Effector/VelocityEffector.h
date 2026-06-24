#pragma once

#include <Arduino.h>
#include "AbstractEffector.h"
#include "BezierCurve.h"
#include "FixedBezierCurve.h"
#include "../../BottangoArduinoConfig.h"
#include "../Communication/BasicCommands.h"
#include "../System/Time.h"

class VelocityEffector : public AbstractEffector
{
public:
	VelocityEffector(int minSignal, int maxSignal, int maxSignalPerSec, int startingSignal);
	virtual void updateSignalBounds(int minSignal, int maxSignal, int signalSpeed) override;
	virtual void updateOnLoop() override;
	virtual void driveOnLoop() override;
	virtual void setSync(int syncValue) override;
	virtual void setAutoSync(int syncValue) override;
	virtual void updateSync(int delta);
	virtual bool useFloatCurve() override;

	virtual void setHome() override;
	virtual void resetHome() override;

protected:
	// for speed limiting
	unsigned long minMicrosPerSignal = 0;

	bool homed = false;

	// for driving
	int targetSignal = 0;
	int currentSignal = 0;
	int autoSync = 0;
	int sync = 0;
	bool postAutoSyncInProgress = false;
	int drive = 0;
	unsigned int signalChangePeriodUs = 0;

	// for timing
	unsigned long lastSignalChangeTimeUs = 0;
	unsigned long sleepStartTime = 0;
	unsigned long periodAbortTime = 0;

	byte inProgressCurveIdx;

	bool updateDrive(unsigned long nowUS, unsigned long currentTimeMs);
	void notifyEndAutoSync();
};