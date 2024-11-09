#pragma once

#include <tuple>

class Point3D
{
public:
	static const Point3D Empty;

	//Point3D& operator=(const Point3D& that)
	//{
	//	if (this != &that)
	//	{
	//		X = that.X;
	//		Y = that.Y;
	//		Z = that.Z;
	//	}
	//	return *this;
	//}

	constexpr bool operator==(const Point3D& that) const { return X == that.X && Y == that.Y && Z == that.Z; }
	constexpr bool operator!=(const Point3D& that) const { return X != that.X && Y != that.Y && Z != that.Z; }

	constexpr Point3D operator+() const { return {+X, +Y, +Z};}
	constexpr Point3D operator-() const { return {-X, -Y, -Z};}

	constexpr Point3D operator+(const Point3D& that) const { return {X + that.X, Y + that.Y, Z + that.Z};}
	constexpr Point3D& operator+=(const Point3D& that) { X += that.X; Y += that.Y; Z += that.Z; return *this; }

	constexpr Point3D operator-(const Point3D& that) const { return {X - that.X, Y - that.Y, Z - that.Z};}
	constexpr Point3D& operator-=(const Point3D& that) { X -= that.X; Y -= that.Y; Z -= that.Z; return *this; }

	constexpr Point3D operator*(const Point3D& that) const { return {X * that.X, Y * that.Y, Z * that.Z};}
	constexpr Point3D operator*=(const Point3D& that) { X *= that.X; Y *= that.Y; Z *= that.Z; return *this; }
	constexpr Point3D operator*(int factor) const { return {X * factor, Y * factor, Z * factor};}
	constexpr Point3D& operator*=(int factor) { X *= factor; Y *= factor; Z *= factor; return *this; }

	constexpr Point3D operator%(const Point3D& that) const { return {X / that.X, Y / that.Y, Z / that.Z};}
	constexpr Point3D operator%=(const Point3D& that) { X /= that.X; Y /= that.Y; Z /= that.Z; return *this; }
	constexpr Point3D operator%(int factor) const { return {X / factor, Y / factor, Z / factor};}
	constexpr Point3D& operator%=(int factor) { X /= factor; Y /= factor; Z /= factor; return *this; }

	constexpr Point3D operator&(const Point3D& that) const { return {X & that.X, Y & that.Y, Z & that.Z};}
	constexpr Point3D operator&=(const Point3D& that) { X &= that.X; Y &= that.Y; Z &= that.Z; return *this; }
	constexpr Point3D operator&(int factor) const { return {X & factor, Y & factor, Z & factor};}
	constexpr Point3D& operator&=(int factor) { X &= factor; Y &= factor; Z &= factor; return *this; }

	constexpr bool IsValid() const { return *this != (Point3D::Empty); }

//=============================Special cases=========================================
	constexpr double powXY() const {
		return double(X * X) + double(Y * Y);
	}

	inline double LengthXY() const {
		return std::sqrt(this->powXY());
	}

	inline double DistanceFromXY(const Point3D& that) const{
		return (that - *this).LengthXY();
	}

	constexpr double DistanceFromSquaredXY(const Point3D& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr double pow() const {
		return double(X * X) + double(Y * Y) + double (Z * Z);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Point3D& that) const{
		return (that - *this).Length();
	}

	constexpr double DistanceFromSquared(const Point3D& that) const {
		return (that - *this).pow();
	}

public:
	int X, Y, Z;
};