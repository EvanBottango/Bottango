#ifndef AbstractCommandStreamDataSource_h
#define AbstractCommandStreamDataSource_h

class AbstractCommandStreamDataSource
{
public:
    virtual void getNextCommand(char *output, bool shouldLoop, unsigned long &msEndOfThisCommand, unsigned long &msStartOfNextCommand);
    virtual void reset();
    virtual void update(bool shouldLoop);
    bool dataComplete = false;
};

#endif