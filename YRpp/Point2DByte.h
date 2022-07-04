#pragma once

#include <utility>

class Point2DBYTE
{
public:
	static const Point2DBYTE Empty;

	Point2DBYTE& operator=(const Point2DBYTE& that)
	{
		if (this != &that)
		{
			X = that.X;
			Y = that.Y;
		}
		return *this;
	}

	bool operator==(const Point2DBYTE& that) const { return X == that.X && Y == that.Y; }
	bool operator!=(const Point2DBYTE& that) const { return X != that.X && Y != that.Y; }

	bool operator>(const Point2DBYTE& that) const { return X > that.X || X == that.X && Y > that.Y; }
	bool operator>=(const Point2DBYTE& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	bool operator<(const Point2DBYTE& that) const { return X < that.X || X == that.X && Y < that.Y; }
	bool operator<=(const Point2DBYTE& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	__forceinline bool operator!() const
	{
		return (*this == Point2DBYTE::Empty);
	}

	__forceinline operator bool() const
	{
		return !(*this == Point2DBYTE::Empty);
	}

public:
	const int DistanceFrom(Point2DBYTE const& nThat) {
		return abs((nThat.X - X) * (nThat.X - X)) + abs((nThat.Y - Y) * (nThat.Y - Y));
	}

	int Length() const {
		return static_cast<int>(Math::sqrt(static_cast<double>(this->X * this->X + this->Y * this->Y)));
	}

	BYTE X, Y;
};