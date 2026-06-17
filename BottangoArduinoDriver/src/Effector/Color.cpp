#include "Color.h"

Color::Color(byte r, byte g, byte b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color()
{
    this->r = 0;
    this->g = 0;
    this->b = 0;
}