#ifndef CommandStream_h
#define CommandStream_h

class CommandStream
{

public:
    CommandStream(const char *charStream, unsigned long streamDurationInMS);
    CommandStream(const char *charStream, unsigned long streamDurationInMS, const char *loopCharStream, unsigned long loopStreamDurationInMS);

    void getNextCommand(char *output);
    bool readyForNextCommand();
    bool complete();
    void reset();
    void setShouldLoop();

private:
    const char *charStream;
    unsigned long streamLength = 0;
    unsigned long streamDurationInMS = 0;

    const char *loopCharStream;
    unsigned long loopStreamLength = 0;
    unsigned long loopStreamDurationInMS = 0;

    unsigned long travel = 0;
    bool shouldLoop = false;
    bool onLoop = false;
};
#endif