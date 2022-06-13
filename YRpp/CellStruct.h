#pragma once

#include <YRMath.h>
#include <utility>

class CellStruct
{
public:
	short X;
	short Y;

	static const CellStruct Empty;

	CellStruct(short nX , short nY) : X { nX } , Y { nY }
	{ }

	CellStruct() : X { 0 }, Y { 0 }
	{ }

	auto operator()()
	{
		// returns a tuple to make it work with std::tie
		return std::make_pair(X, Y);
	}

	//equality
	bool operator==(const CellStruct& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	inline bool operator!=(const CellStruct& a) const
	{
		return (X != a.X || Y != a.Y);
	}

	bool operator!() const
	{
		return (*this == CellStruct::Empty);
	}

	__forceinline operator bool() const
	{
		return !(*this == CellStruct::Empty);
	}

	//magnitude
	double Magnitude_() const
	{
		return Math::sqrt(MagnitudeSquared());
	}

	double Magnitude()
	{
		return sqrt(MagnitudeSquared());
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
		return CellStruct{ (X - a.X), (Y - a.Y) };
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
		return CellStruct { X + a.X, Y + a.Y };
	}
	//addition
	CellStruct& operator+=(const CellStruct& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}

	//and do sqrt when needed only
	double DistanceFromAutoMethod(const CellStruct& a, bool nQuick = false) const
	{
		if (nQuick)
			return Quick_Distance(a);

		return DistanceFrom(a);
	}

	double Quick_Distance(const CellStruct& b) const
	{
		CellStruct buffer = *this;
		double x_diff = abs(static_cast<double>(buffer.X - b.X));
		double y_diff = abs(static_cast<double>(buffer.Y - b.Y));

		if (x_diff > y_diff)
		{
			return (y_diff / 2) + x_diff;
		}
		else
		{
			return (x_diff / 2) + y_diff;
		}
	}
};