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

	int X, Y, Z;
};