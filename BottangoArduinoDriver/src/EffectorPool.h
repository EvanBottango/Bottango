#ifndef EffectorPool_h
#define EffectorPool_h

#include "AbstractEffector.h"
#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"
#include "Module Handling/ModuleLoop.h"

class EffectorPool : public LoopModule
{

public:
    EffectorPool();

	void onPhase(Phase p) override;

    void addEffector(AbstractEffector *effector);

	/**
	 * @brief Template function to add an effector of type T with constructor arguments Args
	 * @tparam T AbstractEffector derived type
	 * @tparam ...Args Argument types for T's constructor
	 * @param ...args Arguments to pass to T's constructor
	 */
	template <typename T, typename... Args>
	void addEffector(Args&&... args)
	{
		//static_assert(std::is_base_of<AbstractEffector, T>::value, "T must derive from AbstractEffector");

		T* newEffector = new T(args...);
		addEffector(newEffector);
	}

    void removeEffector(char *identifier);

    void addCurveToEffector(char *identifier, Curve *curve);

    void updateEffectorSignalBounds(char *identifier, int minSignal, int maxSignal, int signalSpeed);

    void syncEffector(char *identifier, int syncValue);

    void autoSyncEffector(char *identifer, int direction);

    void homeEffector(char *identifier);

    void resetHomeEffector(char *identifier);

    void clearCurvesForEffector(char *identifier);

    void updateAllDriveTargets();

    void deregisterAll();

    void clearAllCurves();

    bool effectorUsesFloatCurve(char *identifier);

    CircularArray<AbstractEffector> effectors = CircularArray<AbstractEffector>(MAX_REGISTERED_EFFECTORS);

    AbstractEffector *getEffector(char *identifier);
};

#endif // BOTTANGOARDUINO_SERVOPOOL_H
