#ifndef BOTTANGOARDUINO_CURVE_H
#define BOTTANGOARDUINO_CURVE_H

class Curve
{
public:
    Curve();
    virtual unsigned long getStartTimeMs() = 0;
};
#endif