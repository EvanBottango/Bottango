#pragma once

#include <Arduino.h>

class Color
{

public:
	constexpr Color() : r(0), g(0), b(0)
	{
		// Empty constructor for default initialization
	}

	constexpr Color(byte r, byte g, byte b) : r(r), g(g), b(b)
	{
		// Empty constructor for initialization with RGB values
	}

    byte r;
    byte g;
    byte b;

	constexpr bool operator==(const Color& other) const
	{
		return r == other.r && g == other.g && b == other.b;
	}
};