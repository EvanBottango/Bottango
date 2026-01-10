// I2SAudioEffector.h
#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#ifndef I2SAudioEffector_h
#define I2SAudioEffector_h

#include "AbstractEffector.h"
#include "SDCardUtil.h"
#include "../BottangoArduinoConfig.h"
#include "Interfaces/AudioPlayback.h"

class I2SAudioEffector : public AbstractEffector
{
public:
    I2SAudioEffector(char* identifier, char* hash, IAudioPlayback* interface);
	//I2SAudioEffector(char** args);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void clearCurves() override;

protected:
private:
	IAudioPlayback* AudioInterface;

    char myIdentifier[9];
    bool shouldFire = false;
	uint32_t offsetMS = 0;
};

#endif
#endif