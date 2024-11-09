#pragma once

#include <cmath>

class Point2D
{
public:
	static const Point2D Empty;

	//Point2D& operator=(const Point2D& that)
	//{
	//	if (this != &that)
	//	{
	//		X = that.X;
	//		Y = that.Y;
	//	}
	//	return *this;
	//}


	constexpr bool operator==(const Point2D& that) const { return X == that.X && Y == that.Y; }
	constexpr bool operator!=(const Point2D& that) const { return X != that.X && Y != that.Y; }

	constexpr bool IsEmpty() const
	{
		return (*this) == Empty;
	}

	constexpr Point2D& operator++() {
		++X;
		++Y;
		return *this;
	}

	constexpr Point2D& operator--() {
		--X;
		--Y;
		return *this;
	}

	constexpr Point2D operator++(int) { Point2D orig = *this; ++(*this); return orig; }
	constexpr Point2D operator--(int) { Point2D orig = *this; --(*this); return orig; }

	constexpr Point2D operator+() const { return {+X, +Y};}
	constexpr Point2D operator-() const { return {-X, -Y};}

	constexpr Point2D operator+(const Point2D& that) const { return {X + that.X, Y + that.Y};}
	constexpr Point2D& operator+=(const Point2D& that) { X += that.X; Y += that.Y; return *this; }
	constexpr Point2D& operator+=(int factor) { X += factor; Y += factor; return *this; }

	constexpr Point2D operator-(const Point2D& that) const { return {X - that.X, Y - that.Y};}
	constexpr Point2D& operator-=(const Point2D& that) { X -= that.X; Y -= that.Y; return *this; }

	constexpr Point2D operator*(const Point2D& that) const { return {X * that.X, Y * that.Y};}
	constexpr Point2D operator*=(const Point2D& that) { X *= that.X; Y *= that.Y; return *this; }
	constexpr Point2D operator*(int factor) const { return {X * factor, Y * factor};}
	constexpr Point2D& operator*=(int factor) { X *= factor; Y *= factor; return *this; }

	constexpr Point2D operator*(double factor) const { return {static_cast<int>(X * factor), static_cast<int>(Y * factor)};}
	constexpr Point2D& operator*=(double factor) { X *= static_cast<int>(factor); Y *= static_cast<int>(factor); return *this; }

	constexpr Point2D operator/(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	constexpr Point2D operator/=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	constexpr Point2D operator/(int factor) const { return {X / factor, Y / factor};}
	constexpr Point2D& operator/=(int factor) { X /= factor; Y /= factor; return *this; }

	constexpr Point2D operator%(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	constexpr Point2D operator%=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	constexpr Point2D operator%(int factor) const { return {X / factor, Y / factor};}
	constexpr Point2D& operator%=(int factor) { X /= factor; Y /= factor; return *this; }

	constexpr Point2D operator&(const Point2D& that) const { return {X & that.X, Y & that.Y};}
	constexpr Point2D operator&=(const Point2D& that) { X &= that.X; Y &= that.Y; return *this; }
	constexpr Point2D operator&(int factor) const { return {X & factor, Y & factor};}
	constexpr Point2D& operator&=(int factor) { X &= factor; Y &= factor; return *this; }

	constexpr bool operator>(const Point2D& that) const { return X > that.X || X == that.X && Y > that.Y; }
	constexpr bool operator>=(const Point2D& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	constexpr bool operator<(const Point2D& that) const { return X < that.X || X == that.X && Y < that.Y; }
	constexpr bool operator<=(const Point2D& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	constexpr bool IsValid() const { return *this != (Point2D::Empty); }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr double pow() const {
		return double(X * X) + double(Y * Y);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Point2D& that) const{
		return (that - *this).Length();
	}

	constexpr double DistanceFromSquared(const Point2D& that) const {
		return (that - *this).pow();
	}
public:

	int X, Y;
};