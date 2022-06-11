#pragma once

#include <Matrix3D.h>
#include <ColorStruct.h>

//used for light colors
struct TintStruct
{
	TintStruct() = default;

	TintStruct(int r, int g, int b) :Red { r }, Green { g }, Blue { b }{}

	int Red, Green, Blue;

	bool operator == (TintStruct const rhs) const
	{
		return Red == rhs.Red && Green == rhs.Green && Blue == rhs.Blue;
	}

	bool operator != (TintStruct const rhs) const
	{
		return !(*this == rhs);
	}

	bool operator < (TintStruct const rhs) const
	{
		if (Red < rhs.Red)
			return true;
		if (Green < rhs.Green)
			return true;
		if (Blue < rhs.Blue)
			return true;
		return false;
	}
};

//Random number range
struct RandomStruct
{
	int Min, Max;
};

//obvious
struct LTRBStruct
{
	int Left;
	int Top;
	int Right;
	int Bottom;
};

//Used In CellClass
struct wRGB
{
	WORD Red, Green, Blue;
};

struct wDimension
{
	WORD width, height;
};