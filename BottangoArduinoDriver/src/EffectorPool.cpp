
#include "EffectorPool.h"
#include "Errors.h"
#include "FreeRam.h"
#include "System/SystemStatus.h"

void EffectorPool::onPhase(Phase const p)
{
	if (p != Phase::Output) return;

	if (SystemStatus::systemStatus.initialized)
	{
		updateAllDriveTargets();
	}
}

void EffectorPool::addEffector(AbstractEffector* effector)
{
	if (effectors.size() >= MAX_REGISTERED_EFFECTORS)
	{
		Error::reportError_NoSpaceAvailable();
		return;
	}

	char inEffectorIdentifier[9];
	effector->getIdentifier(inEffectorIdentifier, 9);

	if (getEffector(inEffectorIdentifier) == NULL)
	{
		effectors.pushBack(effector);
	}
	else
	{
		Error::reportError_EffectorCollision(inEffectorIdentifier);
		return;
	}
}

void EffectorPool::removeEffector(char* identifier)
{
#ifdef __AVR__
	cli(); // stop interrupts
#endif

	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}

	effectors.remove(effector);
	effector->destroy(false);
	delete effector;

#ifdef __AVR__
	sei(); // allow interrupts
#endif
}

void EffectorPool::addCurveToEffector(char* identifier, Curve* curve) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->addCurve(curve);
}

void EffectorPool::updateEffectorSignalBounds(char* identifier, int minSignal, int maxSignal, int signalSpeed) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->updateSignalBounds(minSignal, maxSignal, signalSpeed);
}

void EffectorPool::homeEffector(char* identifier) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->setHome();
}

void EffectorPool::resetHomeEffector(char* identifier) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->resetHome();
}

void EffectorPool::autoSyncEffector(char* identifier, int direction) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->setAutoSync(direction);
}

// this routine is called by the GUI buttons to synch (home) a stepper.  Intercept the values and modify to reflect
// what we specifically want for each stepper motor.

void EffectorPool::syncEffector(char* identifier, int syncValue) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}

	effector->setSync(syncValue);
}

void EffectorPool::clearCurvesForEffector(char* identifier) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->clearCurves();
}

bool EffectorPool::effectorUsesFloatCurve(char* identifier) const
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return true;
	}
	return effector->useFloatCurve();
}

void EffectorPool::updateAllDriveTargets() const
{
	for (byte i = 0; i < effectors.size(); i++)
	{
		effectors.get(i)->updateOnLoop();
		effectors.get(i)->driveOnLoop();
	}
}

void EffectorPool::deregisterAll()
{
#ifdef __AVR__
	cli(); // stop interrupts
#endif

	for (int i = 0; i < effectors.size(); i++)
	{
		AbstractEffector* effector = effectors.get(i);
		effector->destroy(true);
		delete effector;
	}
	effectors.clear();

#ifdef __AVR__
	sei(); // allow interrupts
#endif
}

void EffectorPool::clearAllCurves() const
{
	for (byte i = 0; i < effectors.size(); i++)
	{
		effectors.get(i)->clearCurves();
	}
}

AbstractEffector* EffectorPool::getEffector(char* identifier) const
{
	for (byte i = 0; i < effectors.size(); i++)
	{
		if (effectors.get(i)->respondsToIdentifier(identifier))
		{
			return effectors.get(i);
		}
	}
	return NULL;
}