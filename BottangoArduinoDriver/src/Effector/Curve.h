#pragma once

class Curve
{
public:
    Curve();
    virtual ~Curve() = default;
    virtual unsigned long getStartTimeMs() = 0;
};