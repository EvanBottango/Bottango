#ifndef EffectorPool_h
#define EffectorPool_h

#include "AbstractEffector.h"
#include "CircularArray.h"
#include "../BottangoArduinoConfig.h"
#include "Module Handling/LoopModule.h"

class EffectorPool : public LoopModule
{

public:
    EffectorPool() = default;

	void onPhase(Phase const p) override;

    void addEffector(AbstractEffector *effector);

	/**
	 * @brief Template function to add an effector of type T with constructor arguments Args
	 * @tparam T AbstractEffector derived type
	 * @tparam Args Argument types for T's constructor
	 * @param args Arguments to pass to T's constructor
	 */
	template <typename T, typename... Args>
	void addEffector(Args&&... args)
	{
		//static_assert(std::is_base_of<AbstractEffector, T>::value, "T must derive from AbstractEffector");

		T* newEffector = new T(args...);
		addEffector(newEffector);
	}

    void removeEffector(char *identifier);

    void addCurveToEffector(char *identifier, Curve *curve) const;

    void updateEffectorSignalBounds(char *identifier, int minSignal, int maxSignal, int signalSpeed) const;

    void syncEffector(char *identifier, int syncValue) const;

    void autoSyncEffector(char * identifier, int direction) const;

    void homeEffector(char *identifier) const;

    void resetHomeEffector(char *identifier) const;

    void clearCurvesForEffector(char *identifier) const;

    void updateAllDriveTargets() const;

    void deregisterAll();

    void clearAllCurves() const;

    bool effectorUsesFloatCurve(char *identifier) const;

    CircularArray<AbstractEffector> effectors = CircularArray<AbstractEffector>(MAX_REGISTERED_EFFECTORS);

    AbstractEffector *getEffector(char *identifier) const;
};

#endif // BOTTANGOARDUINO_SERVOPOOL_H
