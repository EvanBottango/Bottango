#ifndef CommandStream_h
#define CommandStream_h

#include "AbstractCommandStreamDataSource.h"

class CommandStream
{

public:
    CommandStream(AbstractCommandStreamDataSource *dataSource);

    void getNextCommand(char *output);
    bool readyForNextCommand();
    bool complete();
    void setShouldLoop();
    ~CommandStream();

private:
    AbstractCommandStreamDataSource *dataSource = nullptr;
    bool shouldLoop = false;
    unsigned long timeOfNextCommand = 0;
    unsigned long msEndOfLatestCommand = 0;
};
#endif