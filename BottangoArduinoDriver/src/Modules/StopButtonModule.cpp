#include "../../BottangoArduinoModules.h"

#ifdef STOP_BUTTON_SUPPORTED

#include "StopButtonModule.h"
#include "../BottangoCore.h"
#include "../Outgoing.h"

void StopButtonModule::init()
{
	pinMode(STOP_BUTTON_PIN, STOP_INPUT_TYPE);
	lastPressTime = 0;
}

void StopButtonModule::onPhase(Phase p)
{
	if (p != Phase::Input) return;

	unsigned long now = millis();
	if (now - lastPressTime <= BUTTON_DEBOUNCE_TIME)
	{
		return;
	}

	if (!isButtonPressed())
	{
		return;
	}

	handleStopAction();
	lastPressTime = now;
}

bool StopButtonModule::isButtonPressed()
{
#ifdef STOP_READ_TYPE_DIGITAL
	return digitalRead(STOP_BUTTON_PIN) == STOP_READ_ACTIVE;

#elif defined(STOP_READ_TYPE_ANALOG)
	int analogVal = analogRead(STOP_BUTTON_PIN);
	return analogVal >= STOP_READ_ACTIVE_MIN &&
		analogVal <= STOP_READ_ACTIVE_MAX;
#else
	return false;
#endif
}

void StopButtonModule::handleStopAction()
{
#ifndef DYNAMIC_STOP_BUTTON_BEHAVIOR
	bool stopIsShutdown = STOP_BUTTON_SHOULD_DISCONNECT;
#else
	bool stopIsShutdown = PersistentConfigUtil::stopIsShutdown();
#endif

	if (BottangoCore::isOffline())
	{
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
		if (stopIsShutdown)
		{
			BottangoCore::stop(true);
		}
		else if (BottangoCore::commandStreamProvider != nullptr)
		{
			BottangoCore::commandStreamProvider->stop();
		}
#endif
	}
	else
	{
		if (stopIsShutdown)
		{
			Outgoing::outgoing_requestEStop();
		}
		else
		{
			Outgoing::outgoing_requestStopPlay();
		}
	}
}

#endif