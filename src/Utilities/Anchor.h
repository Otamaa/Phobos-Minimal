#pragma once

#include "TemplateDef.h"

// Helper class to get anchor points on an arbitrary rectangle or a parallelogram
class Anchor
{
public:
	Valueable<HorizontalPosition> Horizontal { HorizontalPosition::Left };
	Valueable<VerticalPosition>   Vertical { VerticalPosition::Top };

	Anchor() = default;
	Anchor(HorizontalPosition hPos, VerticalPosition vPos)
		: Horizontal { hPos }, Vertical { vPos }
	{
	}

	Anchor(const Anchor& other) = default;
	Anchor& operator=(const Anchor& other) = default;
	~Anchor() = default;

	// Maps enum values to offset relative to width
	constexpr double GetRelativeOffsetHorizontal() const
	{
		// Enum goes from 0 to 2 from left to right. Cast it and divide it
		// by 2 and you get the percentage. Pretty clever huh? - Kerbiter
		return (static_cast<double>(this->Horizontal.Get()) / 2.0);
	}

	// Maps enum values to offset relative to height
	constexpr double GetRelativeOffsetVertical() const
	{
		// Same deal as with the left-right one - Kerbiter
		return (static_cast<double>(this->Vertical.Get()) / 2.0);
	}

	// Get an anchor point for a freeform parallelogram
	constexpr Point2D OffsetPosition(
		const Point2D& topLeft,
		const Point2D& topRight,
		const Point2D& bottomLeft
	) const
	{
		Point2D result { topLeft };
		Point2D deltaTopRight { topRight - topLeft };
		Point2D deltaBottomLeft { bottomLeft - topLeft };

		result += (deltaTopRight * this->GetRelativeOffsetHorizontal());
		result += (deltaBottomLeft * this->GetRelativeOffsetVertical());

		return result;
	}

	constexpr Point2D OffsetPosition(const RectangleStruct& rect) const
	{
		Point2D result { rect.X, rect.Y };

		result.X += static_cast<int>(rect.Width * this->GetRelativeOffsetHorizontal());
		result.Y += static_cast<int>(rect.Height * this->GetRelativeOffsetVertical());

		return result;
	}

	constexpr Point2D OffsetPosition(const LTRBStruct& ltrb) const
	{
		Point2D result { ltrb.Left, ltrb.Top };
		int deltaX = ltrb.Right - ltrb.Left;
		int deltaY = ltrb.Bottom - ltrb.Top;

		result.X += static_cast<int>(deltaX * this->GetRelativeOffsetHorizontal());
		result.Y += static_cast<int>(deltaY * this->GetRelativeOffsetVertical());

		return result;
	}

	void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<Anchor*>(this)->Serialize(Stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->Horizontal)
			.Process(this->Vertical)
			.Success()
			//&& stm.RegisterChange(this)
			; // announce this type
	}
};