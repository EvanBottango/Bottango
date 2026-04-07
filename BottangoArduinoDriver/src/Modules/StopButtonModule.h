#include "../../BottangoArduinoModules.h"

#ifdef STOP_BUTTON_SUPPORTED

#ifndef _StopButtonModule_h
#define _StopButtonModule_h

#include <Arduino.h>
#include "../Module Handling/ModuleMaster.h"

/**
 * @brief Module that handles a physical stop button. Depending on the configuration, it stops all movements or disconnects from Bottango.
 * 
 * @detail This module checks the state of a configured stop button during the Input phase of the main loop.
 * The input can be a digital or a analog read, depending on the setup.
 * If the button is pressed, it triggers a stop action. This stop action can either stop all ongoing movements, or disconnect communcation to Bottango.
 * The button triggers its action immediatley when pressed, regardless of the current loop phase.
 */
class StopButtonModule : public LoopModule
{
public:
	void init() override;
	void onPhase(Phase p) override;

private:
	unsigned long lastPressTime = 0;

	/**
	 * @brief Checks if the stop button is currently pressed.
	 * 
	 * @return true if the button is pressed, false otherwise.
	 */
	bool isButtonPressed() const;

	/**
	 * @brief Handles the action to be taken when the stop button is pressed.
	 * Depending on configuration, it either stops all movements or disconnects from Bottango.
	 */
	void handleStopAction();

};


#endif
#endif