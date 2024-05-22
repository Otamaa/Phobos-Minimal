#pragma once
#include <Dimensions.h>

struct SWRange
{
	constexpr SWRange() : WidthOrRange(-1.0f), Height(-1) { }
	constexpr SWRange(float widthOrRange, int height) : WidthOrRange(widthOrRange), Height(height) { }
	constexpr SWRange(int widthOrRange, int height) : WidthOrRange((float)widthOrRange), Height(height) { }
	constexpr SWRange(const Dimensions& dim) : WidthOrRange((float)dim.Width), Height(dim.Height) { }
	constexpr SWRange(int widthOrRange) : WidthOrRange((float)widthOrRange), Height(-1) { }

	constexpr ~SWRange() = default;

	constexpr SWRange(const SWRange& other) = default;
	constexpr SWRange& operator=(const SWRange& other) = default;

	constexpr float range() const
	{
		return this->WidthOrRange;
	}

	constexpr int width() const
	{
		return static_cast<int>(this->WidthOrRange);
	}

	constexpr int height() const
	{
		return this->Height;
	}

	constexpr Dimensions GeetDimension() const {
		return { this->width(), this->height() };
	}

	constexpr bool empty() const
	{
		return this->WidthOrRange < 0.0
			&& this->Height < 0;
	}

public:

	float WidthOrRange;
	int Height;
};