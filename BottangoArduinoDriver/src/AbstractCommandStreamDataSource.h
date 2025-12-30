#ifndef AbstractCommandStreamDataSource_h
#define AbstractCommandStreamDataSource_h

class AbstractCommandStreamDataSource
{
public:
    virtual void getNextCommand(char *output, bool shouldLoop, bool peek);
    virtual void reset();
    virtual void updateOnLoop() {}
    virtual ~AbstractCommandStreamDataSource();
    bool dataComplete = false;
};

#endif