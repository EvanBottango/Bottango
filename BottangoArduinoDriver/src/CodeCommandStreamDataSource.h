#ifndef CodeCommandStreamDataSource_h
#define CodeCommandStreamDataSource_h

#include "AbstractCommandStreamDataSource.h"
#include <Arduino.h>

class CodeCommandStreamDataSource : public AbstractCommandStreamDataSource
{
public:
    CodeCommandStreamDataSource(const char *const *dataArray, int8_t arrayLength);
    CodeCommandStreamDataSource(const char *const *dataArray, int8_t arrayLength, const char *loopCharStream);
    virtual void getNextCommand(char *output, bool shouldLoop, unsigned long &msEndOfThisCommand, unsigned long &msStartOfNextCommand) override;
    virtual void reset() override;

private:
    void incrementArrayIndex(bool shouldLoop);
    void internalGetCharCommand(char *output, bool shouldLoop, bool persistTravel);
    const char *const *dataArray;
    const char *loopCharStream;

    int8_t arrayLength = 0;

    int8_t dataArrayIndex = 0;
    unsigned int currentDataStringLength = 0;
    unsigned int travel = 0;
};

#endif