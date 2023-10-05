#pragma once

#include <tuple>

class Point3D
{
public:
	static const Point3D Empty;

	Point3D& operator=(const Point3D& that)
	{
		if (this != &that)
		{
			X = that.X;
			Y = that.Y;
			Z = that.Z;
		}
		return *this;
	}

	bool operator==(const Point3D& that) const { return X == that.X && Y == that.Y && Z == that.Z; }
	bool operator!=(const Point3D& that) const { return X != that.X && Y != that.Y && Z != that.Z; }

	Point3D operator+() const { return {+X, +Y, +Z};}
	Point3D operator-() const { return {-X, -Y, -Z};}

	Point3D operator+(const Point3D& that) const { return {X + that.X, Y + that.Y, Z + that.Z};}
	Point3D& operator+=(const Point3D& that) { X += that.X; Y += that.Y; Z += that.Z; return *this; }

	Point3D operator-(const Point3D& that) const { return {X - that.X, Y - that.Y, Z - that.Z};}
	Point3D& operator-=(const Point3D& that) { X -= that.X; Y -= that.Y; Z -= that.Z; return *this; }

	Point3D operator*(const Point3D& that) const { return {X * that.X, Y * that.Y, Z * that.Z};}
	Point3D operator*=(const Point3D& that) { X *= that.X; Y *= that.Y; Z *= that.Z; return *this; }
	Point3D operator*(int factor) const { return {X * factor, Y * factor, Z * factor};}
	Point3D& operator*=(int factor) { X *= factor; Y *= factor; Z *= factor; return *this; }

	Point3D operator%(const Point3D& that) const { return {X / that.X, Y / that.Y, Z / that.Z};}
	Point3D operator%=(const Point3D& that) { X /= that.X; Y /= that.Y; Z /= that.Z; return *this; }
	Point3D operator%(int factor) const { return {X / factor, Y / factor, Z / factor};}
	Point3D& operator%=(int factor) { X /= factor; Y /= factor; Z /= factor; return *this; }

	Point3D operator&(const Point3D& that) const { return {X & that.X, Y & that.Y, Z & that.Z};}
	Point3D operator&=(const Point3D& that) { X &= that.X; Y &= that.Y; Z &= that.Z; return *this; }
	Point3D operator&(int factor) const { return {X & factor, Y & factor, Z & factor};}
	Point3D& operator&=(int factor) { X &= factor; Y &= factor; Z &= factor; return *this; }

	inline bool IsValid() const { return *this != (Point3D::Empty); }

//=============================Special cases=========================================
	inline double powXY() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2);
	}

	inline double LengthXY() const {
		return std::sqrt(this->powXY());
	}

	inline double DistanceFromXY(const Point3D& that) const{
		return (that - *this).LengthXY();
	}

	inline double DistanceFromSquaredXY(const Point3D& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	inline double pow() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2) + (double)std::pow(Z,2);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Point3D& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Point3D& that) const {
		return (that - *this).pow();
	}

public:
	int X, Y, Z;
};