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
    bool internalSDMounted = false;
    bool cardLost = false;
    unsigned long lastMountAttemptTime = 0;

#ifdef ESP32
    SemaphoreHandle_t sdMutex = nullptr;
    SPIClass spi = SPIClass(VSPI);
#endif

    namespace
    {
        SDFileError ensureSDMounted()
        {
            // if has previously mounted, verify it
            if (internalSDMounted)
            {
                File root = SD.open("/");
                if (!root || !root.isDirectory())
                {
                    // can't open, therefore SD is gone
                    root.close();
                    internalSDMounted = false;
                    cardLost = true;
                    lastMountAttemptTime = millis();
#ifdef ENABLE_STATUS_LIGHTS
                    StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR);
#endif
                    Outgoing::printOutputStringFlash(F("SD Removed"));
                    Outgoing::printLine();

                    return SDFileError::ERR_NO_CARD;
                }
                root.close();
                return SDFileError::ERR_NONE;
            }

            // not previously mounted, so throttle how fast we can retry
            if (lastMountAttemptTime > 0 && millis() - lastMountAttemptTime < SD_CARD_REMOUNT_TIME)
            {
                Outgoing::printOutputStringFlash(F("No SD"));
                Outgoing::printLine();
                return SDFileError::ERR_NO_CARD;
            }

#ifdef SD_PIN_HIGH
            // power-on sequence if needed
            digitalWrite(SD_EN_PIN, HIGH);
            pinMode(SD_EN_PIN, OUTPUT);
#endif

            lastMountAttemptTime = millis();
#ifdef ESP32
            spi.begin(SDPIN_CLK, SDPIN_MISO, SDPIN_MOSI, SDPIN_CS);
            // reset if we lost card before restarting
            if (cardLost)
            {
                SD.end();
                cardLost = false;
            }
            // try and mount
            if (!SD.begin(SDPIN_CS, spi, 25000000))
            {
#ifdef ENABLE_STATUS_LIGHTS
                StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR);
#endif
                Outgoing::printOutputStringFlash(F("Can't Mount SD"));
                Outgoing::printLine();
                return SDFileError::ERR_NO_CARD;
            }
#else
            if (!SD.begin(SDPIN_CS))
            {
#ifdef ENABLE_STATUS_LIGHTS
                StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_NOSD);
#endif
                Outgoing::printOutputStringFlash(F("Can't Mount SD"));
                Outgoing::printLine();
                return SDFileError::ERR_NO_CARD;
            }
#endif

            internalSDMounted = true;
            return SDFileError::ERR_NONE;
        }
    }

    File openFile(const char *filePath, SDFileError &fileError)
    {
        lockCard();
        fileError = ensureSDMounted();
        if (fileError != SDFileError::ERR_NONE)
        {
            // error case
            unlockCard();
            return File();
        }

        File file = SD.open(filePath);
        if (!file)
        {
#ifdef ENABLE_STATUS_LIGHTS
            StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR);
#endif

            fileError = SD.exists(filePath) ? SDFileError::ERR_IO : SDFileError::ERR_FILE_NOT_FOUND;
            if (fileError == SDFileError::ERR_IO)
            {
                Outgoing::printOutputStringFlash(F("IO SD fail"));
                Outgoing::printLine();
            }
            else if (fileError == SDFileError::ERR_FILE_NOT_FOUND)
            {
                Outgoing::printOutputStringFlash(F("No File"));
                Outgoing::printLine();
            }

            unlockCard();
            return File();
        }

        fileError = SDFileError::ERR_NONE;
        unlockCard();
        return file;
    }

    File openFileForWrite(const char *filePath, SDFileError &fileError)
    {
        lockCard();
        fileError = ensureSDMounted();
        if (fileError != SDFileError::ERR_NONE)
        {
            // error case
            unlockCard();
            return File();
        }

#ifdef ESP32
        // On ESP32 SD implementation FILE_WRITE == "w" means create or truncate
        File file = SD.open(filePath, FILE_WRITE);
#else
        // other platforms FILE_WRITE usually appends, so remove first
        if (SD.exists(filePath) && !SD.remove(filePath))
        {
            fileError = SDFileError::ERR_IO;
            unlockCard();
            return File();
        }
        File file = SD.open(filePath, FILE_WRITE);
#endif

        if (!file)
        {
            fileError = SD.exists(filePath) ? SDFileError::ERR_IO : SDFileError::ERR_FILE_NOT_FOUND;
        }
        else
        {
            fileError = SDFileError::ERR_NONE;
        }
        unlockCard();
        return file;
    }

    bool fileExists(const char *filePath, SDFileError &fileError)
    {
        lockCard();
        fileError = ensureSDMounted();
        if (fileError == SDFileError::ERR_NO_CARD)
        {
            unlockCard();
            return false;
        }
        bool exists = SD.exists(filePath);
        if (!exists)
        {
            fileError = SDFileError::ERR_FILE_NOT_FOUND;
        }
        else
        {
            fileError = SDFileError::ERR_NONE;
        }
        unlockCard();
        return exists;
    }

    // Write a chunk to an already-open file.
    // Returns bytes written or -1 on error.
    ssize_t writeChunk(File &file, const uint8_t *data, size_t len, SDFileError &fileError)
    {
        lockCard();
        if (!file)
        {
            fileError = SDFileError::ERR_NO_CARD;
            unlockCard();
            return -1;
        }
        ssize_t bytesWritten = file.write(data, len);
        fileError = (bytesWritten < 0) ? SDFileError::ERR_IO : SDFileError::ERR_NONE;
        unlockCard();
        return bytesWritten;
    }

    // Overload for C-strings
    inline ssize_t writeChunk(File &file, const char *str, SDFileError &fileError)
    {
        return writeChunk(file, reinterpret_cast<const uint8_t *>(str), strlen(str), fileError);
    }

    void closeFile(File &file)
    {
        lockCard();
        if (file)
        {
            file.flush();
            file.close();
        }
        unlockCard();
    }

    bool safeAvailable(File &file)
    {
        lockCard();
        bool result = false;
        if (file)
        {
            result = file.available();
        }
        unlockCard();
        return result;
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

    void getAudioFilePath(char *identifier, char *output)
    {

        // add audio path
        strcpy_P(output, SD_AUDIO_PATH); // "/audio/"

        // add identifier string (8 char hex)
        strcat(output, identifier); // "/audio/FFFFFFFF"

        // add loop or data suffix
        char workingString[20];
        workingString[0] = '\0';
        strcpy_P(workingString, SD_AUDIO_FORMAT); // .wav

        strcat(output, workingString); // "/audio/FFFFFFFF.wav"
    }

    void getAudioHashFilePath(char *identifier, char *output)
    {

        // add audio path
        strcpy_P(output, SD_AUDIO_PATH); // "/audio/"

        // add identifier string (8 char hex)
        strcat(output, identifier); // "/audio/FFFFFFFF"

        // add loop or data suffix
        char workingString[20];
        workingString[0] = '\0';
        strcpy_P(workingString, SD_HASH_FORMAT); // hash.txt

        strcat(output, workingString); // "/audio/FFFFFFFFhash.txt"
    }
}
#endif