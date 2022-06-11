#pragma once

class Point2D
{
public:
	static const Point2D Empty;

	Point2D operator+(const Point2D& a) const
	{
		return Point2D{ X + a.X, Y + a.Y };
	}

	Point2D& operator+=(const Point2D& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}

	//equality
	bool operator==(const Point2D& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	inline bool operator!=(const Point2D& a) const
	{
		return (X != a.X || Y != a.Y);
	}

	bool operator!() const
	{
		return (*this == Point2D::Empty);
	}

	__forceinline operator bool() const
	{
		return !(*this == Point2D::Empty);
	}

	//substraction
	Point2D operator-(const Point2D& a) const
	{
		return Point2D { X - a.X, Y - a.Y };
	}
	//substraction
	Point2D& operator-=(const Point2D& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}

	int Length() const {
		JMP_THIS(0x4C1B50);
	}

	int X, Y;
};