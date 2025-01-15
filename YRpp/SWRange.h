#pragma once
#include <Dimensions.h>

struct SWRange
{
	COMPILETIMEEVAL SWRange() : WidthOrRange(-1.0f), Height(-1) { }
	COMPILETIMEEVAL SWRange(float widthOrRange, int height) : WidthOrRange(widthOrRange), Height(height) { }
	COMPILETIMEEVAL SWRange(int widthOrRange, int height) : WidthOrRange((float)widthOrRange), Height(height) { }
	COMPILETIMEEVAL SWRange(const Dimensions& dim) : WidthOrRange((float)dim.Width), Height(dim.Height) { }
	COMPILETIMEEVAL SWRange(int widthOrRange) : WidthOrRange((float)widthOrRange), Height(-1) { }

	COMPILETIMEEVAL ~SWRange() = default;

	COMPILETIMEEVAL SWRange(const SWRange& other) = default;
	COMPILETIMEEVAL SWRange& operator=(const SWRange& other) = default;

	COMPILETIMEEVAL float range() const
	{
		return this->WidthOrRange;
	}

	COMPILETIMEEVAL int width() const
	{
		return static_cast<int>(this->WidthOrRange);
	}

	COMPILETIMEEVAL int height() const
	{
		return this->Height;
	}

	COMPILETIMEEVAL Dimensions GeetDimension() const {
		return { this->width(), this->height() };
	}

	COMPILETIMEEVAL bool empty() const
	{
		return this->WidthOrRange < 0.0
			&& this->Height < 0;
	}

public:

	float WidthOrRange;
	int Height;
};