#pragma once

#include <Matrix3D.h>
#include <ColorStruct.h>

//used for light colors
struct TintStruct
{
	TintStruct() = default;

	TintStruct(int r, int g, int b) noexcept : Red { r }, Green { g }, Blue { b }{}
	TintStruct(const ColorStruct& nColor ,  double nTintFactor) noexcept :
		Red { 0 }, Green { 0 }, Blue { 0 }
	{
		auto nValR = ((1000 * nColor.R / 255) * nTintFactor);
		Red = static_cast<int>(MinImpl(nValR, 2000.0));
		auto nValG = ((1000 * nColor.G / 255) * nTintFactor);
		Green = static_cast<int>(MinImpl(nValG, 2000.0));
		auto nValB = ((1000 * nColor.B / 255) * nTintFactor);
		Blue = static_cast<int>(MinImpl(nValB, 2000.0));
	}

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

public:

	int Red, Green, Blue;
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