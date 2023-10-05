#pragma once
#include <Dimensions.h>

struct SWRange
{
	SWRange() : WidthOrRange(-1.0f), Height(-1) { }
	SWRange(float widthOrRange, int height) : WidthOrRange(widthOrRange), Height(height) { }
	SWRange(int widthOrRange, int height) : WidthOrRange((float)widthOrRange), Height(height) { }
	SWRange(const Dimensions& dim) : WidthOrRange((float)dim.Width), Height(dim.Height) { }
	SWRange(int widthOrRange) : WidthOrRange((float)widthOrRange), Height(-1) { }

	~SWRange() = default;

	SWRange(const SWRange& other) = default;
	SWRange& operator=(const SWRange& other) = default;

	float range() const
	{
		return this->WidthOrRange;
	}

	int width() const
	{
		return static_cast<int>(this->WidthOrRange);
	}

	int height() const
	{
		return this->Height;
	}

	Dimensions GeetDimension() const {
		return { this->width(), this->height() };
	}

	bool empty() const
	{
		return this->WidthOrRange < 0.0
			&& this->Height < 0;
	}

public:

	float WidthOrRange;
	int Height;
};