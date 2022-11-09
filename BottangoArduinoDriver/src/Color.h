
#ifndef BOTTANGOARDUINO_COLOR_H
#define BOTTANGOARDUINO_COLOR_H

#include "Arduino.h"

class Color
{

public:
    Color();
    Color(byte r, byte g, byte b);
    byte r;
    byte g;
    byte b;
};

#endif //BOTTANGOARDUINO_COLOR_H