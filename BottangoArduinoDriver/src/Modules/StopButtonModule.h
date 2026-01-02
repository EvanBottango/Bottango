#include "../../BottangoArduinoModules.h"

#ifdef STOP_BUTTON_SUPPORTED

#ifndef _StopButtonModule_h
#define _StopButtonModule_h

#include <Arduino.h>
#include "../Module Handling/ModuleMaster.h"

class StopButtonModule : public LoopModule
{
public:
	void init() override;
	void onPhase(Phase p) override;

private:
	unsigned long lastPressTime = 0;

	bool isButtonPressed();
	void handleStopAction();

};


#endif
#endif