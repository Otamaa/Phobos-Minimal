#pragma once

#include <YRMath.h>
#include <utility>

class CellStruct
{
public:
	short X { 0 };
	short Y { 0 };

	static const CellStruct Empty;
	static const CellStruct DefaultUnloadCell;

	inline bool SimilarTo(const CellStruct& a) const {return (X == a.X && Y == a.Y); }
	inline bool DifferTo(const CellStruct& a)const { return (X != a.X || Y != a.Y); }
	inline bool IsValid() const { return this->DifferTo(CellStruct::Empty); }

	//equality
	inline bool operator==(const CellStruct& a) const {
		return (X == a.X && Y == a.Y);
	}

	inline DWORD Pack() const noexcept {
		return std::bit_cast<DWORD>(*this);
	}

	double Magnitude()
	{
		return std::sqrt(MagnitudeSquared());
	}

	//magnitude squared
	double MagnitudeSquared() const {
		return static_cast<double>(X) * X + static_cast<double>(Y) * Y;
	}

	//distance from another vector
	double DistanceFrom(const CellStruct& a) const
	{
		return (a - *this).Magnitude();
	}
	//distance from another vector, squared
	double DistanceFromSquared(const CellStruct& a) const
	{
		return (a - *this).MagnitudeSquared();
	}

	//substraction
	CellStruct operator-(const CellStruct& a) const
	{
		return { static_cast<short>(X - a.X),  static_cast<short>(Y - a.Y) };
	}
	//substraction
	CellStruct& operator-=(const CellStruct& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}

	//addition
	CellStruct operator+(const CellStruct& a) const
	{
		return { static_cast<short>(X + a.X), static_cast<short>(Y + a.Y) };
	}
	//addition
	CellStruct& operator+=(const CellStruct& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}

	//and do sqrt when needed only
	template<bool T = false>
	double DistanceFromAutoMethod(const CellStruct& a) const {
		return Distance(a, std::bool_constant<T>::type());
	}

	short Length() const {
		return static_cast<short>(std::sqrt(static_cast<double>(X) * static_cast<double>(X) + static_cast<double>(Y) * static_cast<double>(Y)));
	}

private:
	double Distance(const CellStruct& b, const std::true_type) {
		CellStruct buffer = *this;
		const double x_diff = abs(static_cast<double>(buffer.X - b.X));
		const double y_diff = abs(static_cast<double>(buffer.Y - b.Y));

		if (x_diff > y_diff)
		{
			return (y_diff / 2) + x_diff;
		}
		else
		{
			return (x_diff / 2) + y_diff;
		}
	}

	double Distance(const CellStruct& a, const std::false_type) {
		return (a - *this).Magnitude();
	}
};