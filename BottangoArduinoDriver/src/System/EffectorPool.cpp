
#include "EffectorPool.h"
#include "Errors.h"

EffectorPool::EffectorPool()
{}

void EffectorPool::addEffector(AbstractEffector* inEffector)
{
	if (effectors.size() >= MAX_REGISTERED_EFFECTORS)
	{
		Error::reportError_NoSpaceAvailable();
		return;
	}

	char inEffectorIdentifier[9];
	inEffector->getIdentifier(inEffectorIdentifier, 9);

	AbstractEffector* existingEffector = getEffector(inEffectorIdentifier);

	if (existingEffector == NULL)
	{
		effectors.pushBack(inEffector);
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

void EffectorPool::addCurveToEffector(char* identifier, Curve* curve)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->addCurve(curve);
}

void EffectorPool::updateEffectorSignalBounds(char* identifier, int minSignal, int maxSignal, int signalSpeed)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->updateSignalBounds(minSignal, maxSignal, signalSpeed);
}

void EffectorPool::homeEffector(char* identifier)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->setHome();
}

void EffectorPool::resetHomeEffector(char* identifier)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->resetHome();
}

void EffectorPool::autoSyncEffector(char* identifier, int direction)
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

void EffectorPool::syncEffector(char* identifier, int syncValue)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}

	effector->setSync(syncValue);
}

void EffectorPool::clearCurvesForEffector(char* identifier)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return;
	}
	effector->clearCurves();
}

bool EffectorPool::effectorUsesFloatCurve(char* identifier)
{
	AbstractEffector* effector = getEffector(identifier);
	if (effector == NULL)
	{
		Error::reportError_NoEffectorOnPin(identifier);
		return true;
	}
	return effector->useFloatCurve();
}

void EffectorPool::updateAllDriveTargets()
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
		delete effectors.get(i);
	}
	effectors.clear();

#ifdef __AVR__
	sei(); // allow interrupts
#endif
}

void EffectorPool::clearAllCurves()
{
	for (byte i = 0; i < effectors.size(); i++)
	{
		effectors.get(i)->clearCurves();
	}
}

AbstractEffector* EffectorPool::getEffector(char* identifier)
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