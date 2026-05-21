#include "../BottangoArduinoModules.h"
#if defined(AUDIO_SD_I2S)

#ifndef I2SAudEventStatusResponder_h
#define I2SAudEventStatusResponder_h

#include "AbstractMultiMessageOutgoingSource.h"

// good to go
#define I2S_AUDIO_STATUS_READY 0

// can't play
#define I2S_AUDIO_STATUS_NO_SD_CARD 1
#define I2S_AUDIO_STATUS_NO_FILE_ON_CARD 2
#define I2S_AUDIO_STATUS_BAD_AUDIO_FILE 3
#define I2S_AUDIO_STATUS_NO_HASH_ON_CARD 4

// could play, mismatched hash
#define I2S_AUDIO_STATUS_NO_HASH_MATCH_ON_CARD 5

// -> rAud,ident,hash
// <- OK
// <- sAud,statusCode
// -> OK
// <- OK
// Complete

const char REPLY_AUD_STATUS[] PROGMEM = "sAud";

class I2SAudEventStatusResponder : public AbstractMultiMessageOutgoingSource
{
public:
    I2SAudEventStatusResponder(byte incomingStatus);

    virtual void cleanUpMultiMessage() override; // cleanup if aborting...

    byte status;

private:
    void onMultiMessageStart() override;
    bool emitNextChunk() override;
};

#endif
#endif
