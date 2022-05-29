#ifndef I2CStepperEffector_h
#define I2CStepperEffector_h

#include "LoopDrivenEffector.h"
#include "Arduino.h"
#include "../BottangoArduinoConfig.h"

#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
#include <Adafruit_MotorShield.h>
#endif

class I2CStepperEffector : public LoopDrivenEffector
{
public:
    I2CStepperEffector(byte i2cAddress, byte pin, int maxCounterClockwiseSteps, int maxClockwiseSteps, int maxSignalPerSec, int startingSignal);
    virtual void driveOnLoop() override;

    virtual void getIdentifier(char *outArray, short arraySize) override;
    virtual void destroy() override;
    virtual void dump() override;

    virtual void setSync(int syncValue, bool isTracked) override;

protected:
private:
    byte pin = 0;
    byte i2cAddress = 0;
    int syncValue = 0;
#ifdef USE_ADAFRUIT_MOTOR_SHIELD_V2_LIBRARY
    Adafruit_StepperMotor *stepper;
#endif
};

#endif