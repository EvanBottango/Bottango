#pragma once

#include <Arduino.h>

class Color
{

public:
    Color();
    Color(byte r, byte g, byte b);
    byte r = 0;
    byte g = 0;
    byte b = 0;

	bool operator==(const Color& other) const
	{
		return r == other.r && g == other.g && b == other.b;
	}
};