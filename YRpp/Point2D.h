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

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_pair(X, Y);
	//}

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

	//scalar multiplication
	Point2D operator*(double r) const
	{
		return Point2D { static_cast<int>(X * r), static_cast<int>(Y * r) };
	}

	int Length() const {
		return static_cast<int>(Math::sqrt(static_cast<double>(this->X * this->X + this->Y * this->Y)));
	}

	int X, Y;
};