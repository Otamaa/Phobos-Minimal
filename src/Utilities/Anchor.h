#pragma once

#include "TemplateDef.h"

// Helper class to get anchor points on an arbitrary rectangle or a parallelogram
class Anchor
{
public:
	HorizontalPosition Horizontal { HorizontalPosition::Left };
	VerticalPosition Vertical { VerticalPosition::Top };

	Anchor() = default;
	Anchor(HorizontalPosition hPos, VerticalPosition vPos)
		: Horizontal { hPos }, Vertical { vPos }
	{
	}

	Anchor(const Anchor& other) = default;
	Anchor& operator=(const Anchor& other) = default;
	~Anchor() = default;

	// Maps enum values to offset relative to width
	COMPILETIMEEVAL double GetRelativeOffsetHorizontal() const
	{
		// Enum goes from 0 to 2 from left to right. Cast it and divide it
		// by 2 and you get the percentage. Pretty clever huh? - Kerbiter
		return (static_cast<double>(this->Horizontal) / 2.0);
	}

	// Maps enum values to offset relative to height
	COMPILETIMEEVAL double GetRelativeOffsetVertical() const
	{
		// Same deal as with the left-right one - Kerbiter
		return (static_cast<double>(this->Vertical) / 2.0);
	}

	// Get an anchor point for a freeform parallelogram
	COMPILETIMEEVAL Point2D OffsetPosition(
		const Point2D& topLeft,
		const Point2D& topRight,
		const Point2D& bottomLeft
	) const
	{
		Point2D result { topLeft };

		result += ((topRight - topLeft) * this->GetRelativeOffsetHorizontal());
		result += ((bottomLeft - topLeft) * this->GetRelativeOffsetVertical());

		return result;
	}

	COMPILETIMEEVAL Point2D OffsetPosition(const RectangleStruct& rect) const
	{
		return {
			rect.X + static_cast<int>(rect.Width * this->GetRelativeOffsetHorizontal()),
			rect.Y + static_cast<int>(rect.Height * this->GetRelativeOffsetVertical())
		};
	}

	COMPILETIMEEVAL Point2D OffsetPosition(const LTRBStruct& ltrb) const
	{
		return {
			ltrb.Left + static_cast<int>((ltrb.Right - ltrb.Left) * this->GetRelativeOffsetHorizontal()),
			ltrb.Top + static_cast<int>((ltrb.Bottom - ltrb.Top) * this->GetRelativeOffsetVertical())
		};
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
