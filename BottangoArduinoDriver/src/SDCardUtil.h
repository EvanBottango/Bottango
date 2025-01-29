#include "../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)

#ifndef SDCardUtil_h
#define SDCardUtil_h

#include "Arduino.h"
#include "SDCardUtil.h"
#include <SPI.h>
#include "SD.h"

namespace SDCardUtil
{
    void initialize();
    bool fileExists(const char *filePath);
    File openFile(const char *filePath);
    void closeFile(File file);
    void getAnimationFilePath(byte index, char *output, bool loop, bool config);
    void getAudioFilePath(byte index, char *output);
    void getSetupFilePath(char *output);

    extern bool sdCardInitialized;
    extern bool sdCardAvailable;
#ifdef ESP32
    extern SPIClass spi;
#endif

} // namespace SDCardUtil
#endif
#endif