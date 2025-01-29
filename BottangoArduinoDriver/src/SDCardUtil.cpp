#include "../BottangoArduinoModules.h"
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(AUDIO_SD_I2S)
#include "SDCardUtil.h"
#include "../BottangoArduinoConfig.h"
#include "Outgoing.h"

#ifdef ENABLE_STATUS_LIGHTS
#include "StatusLights.h"
#endif

namespace SDCardUtil
{
    bool sdCardInitialized = false;
    bool sdCardAvailable = false;
#ifdef ESP32
    SPIClass spi = SPIClass(VSPI);
#endif

    void initialize()
    {
        if (sdCardInitialized)
        {
            return;
        }

        sdCardInitialized = true;
#ifdef ESP32
        spi.begin(SDPIN_CLK, SDPIN_MISO, SDPIN_MOSI, SDPIN_CS);
        if (!SD.begin(SDPIN_CS, spi, 25000000))
#else
        if (!SD.begin(SDPIN_CS))
#endif
        {
            Outgoing::printOutputStringFlash(F("SD Card Mount Failed"));
            Outgoing::printLine();
#ifdef ENABLE_STATUS_LIGHTS
            StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_NOSD);
#endif
        }
        else
        {
            sdCardAvailable = true;
        }
    }

    bool fileExists(const char *filePath)
    {
        return SD.exists(filePath);
    }

    File openFile(const char *filePath)
    {
        if (!sdCardInitialized)
        {
            initialize();
        }
        if (!sdCardAvailable)
        {
            return File();
        }

        File file = SD.open(filePath);
        if (!file)
        {
            Outgoing::printOutputStringMem(filePath);
            Outgoing::printOutputStringFlash(F(" : SD Card file not found"));
            Outgoing::printLine();
#ifdef ENABLE_STATUS_LIGHTS
            StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_NOANIM);
#endif
            return file;
        }
        return file;
    }

    void closeFile(File file)
    {
        if (file)
        {
            file.close();
        }
    }

    void getAnimationFilePath(byte index, char *output, bool loop, bool config)
    {
        char workingString[20];

        // add animations path
        strcpy_P(output, SD_ANIMATION_PATH); // "/anims/"

        // add index directory
        workingString[0] = '\0';
        itoa(index, workingString, 10);
        strcat(output, workingString); // "/anims/10"

        strcat(output, "/"); // "/anims/10/"

        // add loop or data suffix
        const char *suffix;
        workingString[0] = '\0';

        if (config)
        {
            suffix = SD_DATA_CONFIGDATA; // "/anims/10/config.txt"
        }
        else if (loop)
        {
            suffix = SD_DATA_LOOPDATA; // "/anims/10/loop.txt"
        }
        else
        {
            suffix = SD_DATA_ANIMDATA; // "/anims/10/data.txt"
        }

        strcpy_P(workingString, suffix);
        strcat(output, workingString);
    }

    void getSetupFilePath(char *output)
    {
        strcpy_P(output, SD_ANIMATION_PATH); // "/anims/"
        strcat_P(output, SD_SETUP_PATH);     // "/anims/setup/"
        strcat_P(output, SD_DATA_ANIMDATA);  // "/anims/setup/data.txt"
    }

    void getAudioFilePath(byte index, char *output)
    {
        char workingString[20];

        // add audio path
        strcpy_P(output, SD_AUDIO_PATH); // "/audio/"

        // add index directory
        workingString[0] = '\0';
        itoa(index, workingString, 10);
        strcat(output, workingString); // "/audio/10"

        // add loop or data suffix
        workingString[0] = '\0';

        strcpy_P(workingString, SD_AUDIO_FORMAT);
        strcat(output, workingString);
    }
}
#endif