#pragma once

#include <cmath>
#include <Base/Always.h>

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


	COMPILETIMEEVAL bool operator==(const Point2D& that) const { return X == that.X && Y == that.Y; }
	COMPILETIMEEVAL bool operator!=(const Point2D& that) const { return X != that.X && Y != that.Y; }

	COMPILETIMEEVAL bool IsEmpty() const
	{
		return (*this) == Empty;
	}

	COMPILETIMEEVAL Point2D& operator++() {
		++X;
		++Y;
		return *this;
	}

	COMPILETIMEEVAL Point2D& operator--() {
		--X;
		--Y;
		return *this;
	}

	COMPILETIMEEVAL Point2D operator++(int) { Point2D orig = *this; ++(*this); return orig; }
	COMPILETIMEEVAL Point2D operator--(int) { Point2D orig = *this; --(*this); return orig; }

	COMPILETIMEEVAL Point2D operator+() const { return {+X, +Y};}
	COMPILETIMEEVAL Point2D operator-() const { return {-X, -Y};}

	COMPILETIMEEVAL Point2D operator+(const Point2D& that) const { return {X + that.X, Y + that.Y};}
	COMPILETIMEEVAL Point2D& operator+=(const Point2D& that) { X += that.X; Y += that.Y; return *this; }
	COMPILETIMEEVAL Point2D& operator+=(int factor) { X += factor; Y += factor; return *this; }

	COMPILETIMEEVAL Point2D operator-(const Point2D& that) const { return {X - that.X, Y - that.Y};}
	COMPILETIMEEVAL Point2D& operator-=(const Point2D& that) { X -= that.X; Y -= that.Y; return *this; }

	COMPILETIMEEVAL Point2D operator*(const Point2D& that) const { return {X * that.X, Y * that.Y};}
	COMPILETIMEEVAL Point2D operator*=(const Point2D& that) { X *= that.X; Y *= that.Y; return *this; }
	COMPILETIMEEVAL Point2D operator*(int factor) const { return {X * factor, Y * factor};}
	COMPILETIMEEVAL Point2D& operator*=(int factor) { X *= factor; Y *= factor; return *this; }

	COMPILETIMEEVAL Point2D operator*(double factor) const { return {static_cast<int>(X * factor), static_cast<int>(Y * factor)};}
	COMPILETIMEEVAL Point2D& operator*=(double factor) { X *= static_cast<int>(factor); Y *= static_cast<int>(factor); return *this; }

	COMPILETIMEEVAL Point2D operator/(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	COMPILETIMEEVAL Point2D operator/=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	COMPILETIMEEVAL Point2D operator/(int factor) const { return {X / factor, Y / factor};}
	COMPILETIMEEVAL Point2D& operator/=(int factor) { X /= factor; Y /= factor; return *this; }

	COMPILETIMEEVAL Point2D operator%(const Point2D& that) const { return {X / that.X, Y / that.Y};}
	COMPILETIMEEVAL Point2D operator%=(const Point2D& that) { X /= that.X; Y /= that.Y; return *this; }
	COMPILETIMEEVAL Point2D operator%(int factor) const { return {X / factor, Y / factor};}
	COMPILETIMEEVAL Point2D& operator%=(int factor) { X /= factor; Y /= factor; return *this; }

	COMPILETIMEEVAL Point2D operator&(const Point2D& that) const { return {X & that.X, Y & that.Y};}
	COMPILETIMEEVAL Point2D operator&=(const Point2D& that) { X &= that.X; Y &= that.Y; return *this; }
	COMPILETIMEEVAL Point2D operator&(int factor) const { return {X & factor, Y & factor};}
	COMPILETIMEEVAL Point2D& operator&=(int factor) { X &= factor; Y &= factor; return *this; }

	COMPILETIMEEVAL bool operator>(const Point2D& that) const { return X > that.X || X == that.X && Y > that.Y; }
	COMPILETIMEEVAL bool operator>=(const Point2D& that) const { return X >= that.X || X == that.X && Y >= that.Y; }

	COMPILETIMEEVAL bool operator<(const Point2D& that) const { return X < that.X || X == that.X && Y < that.Y; }
	COMPILETIMEEVAL bool operator<=(const Point2D& that) const { return X <= that.X || X == that.X && Y <= that.Y; }

	COMPILETIMEEVAL bool IsValid() const { return *this != (Point2D::Empty); }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL double pow() const {
		return double(X * X) + double(Y * Y);
	}

	OPTIONALINLINE double Length() const {
		return sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Point2D& that) const{
		return (that - *this).Length();
	}

	COMPILETIMEEVAL double DistanceFromSquared(const Point2D& that) const {
		return (that - *this).pow();
	}
public:

	int X, Y;
};
