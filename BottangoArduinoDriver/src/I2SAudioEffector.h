#include "../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#ifndef I2SAudioEffector_h
#define I2SAudioEffector_h

#include "AbstractEffector.h"
#include "SDCardUtil.h"
#include "../BottangoArduinoConfig.h"
#include "I2SHelper.h"

class I2SAudioEffector : public AbstractEffector
{
public:
    I2SAudioEffector(char *identifier, byte audioID);

    virtual void updateOnLoop() override;
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void clearCurves() override;

protected:
private:
    char myIdentifier[9];
    bool shouldFire = false;
    unsigned long offsetMS = 0;
    byte audioID = 0;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    i2s_data_bit_width_t i2s_bits_per_sample;
    i2s_slot_mode_t channel_format;
#else
    i2s_bits_per_sample_t i2s_bits_per_sample;
    i2s_channel_fmt_t channel_format;
#endif
#endif
};

#endif
#endif