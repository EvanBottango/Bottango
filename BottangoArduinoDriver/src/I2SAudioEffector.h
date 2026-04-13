#ifndef I2SAudioEffector_h
#define I2SAudioEffector_h

#include "../BottangoArduinoModules.h"
#ifdef AUDIO_SD_I2S

#include "AbstractEffector.h"
#include "DataSource/SDCardUtil.h"
#include "../BottangoArduinoConfig.h"
#include "Modules/Audio/I2SAudioModule.h"

class I2SAudioEffector : public AbstractEffector
{
public:
    I2SAudioEffector(const char* identifier, const char* hash);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void clearCurves() override;

protected:
private:
	I2SAudioModule* _audioModule = nullptr;

    char _myIdentifier[9];
    bool _shouldFire = false;
	uint32_t _offsetMs = 0;
};

#endif
#endif