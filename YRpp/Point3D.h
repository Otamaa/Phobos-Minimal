#pragma once

#include <YRMath.h>
#include <tuple>
#include <span>

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

	COMPILETIMEEVAL void assignto(int(&out)[3]) {
		out[0] = this->X;
		out[1] = this->Y;
		out[2] = this->Z;
	}

	COMPILETIMEEVAL bool operator==(const Point3D& that) const { return X == that.X && Y == that.Y && Z == that.Z; }
	COMPILETIMEEVAL bool operator!=(const Point3D& that) const { return X != that.X && Y != that.Y && Z != that.Z; }

	COMPILETIMEEVAL Point3D operator+() const { return {+X, +Y, +Z};}
	COMPILETIMEEVAL Point3D operator-() const { return {-X, -Y, -Z};}

	COMPILETIMEEVAL Point3D operator+(const Point3D& that) const { return {X + that.X, Y + that.Y, Z + that.Z};}
	COMPILETIMEEVAL Point3D& operator+=(const Point3D& that) { X += that.X; Y += that.Y; Z += that.Z; return *this; }

	COMPILETIMEEVAL Point3D operator-(const Point3D& that) const { return {X - that.X, Y - that.Y, Z - that.Z};}
	COMPILETIMEEVAL Point3D& operator-=(const Point3D& that) { X -= that.X; Y -= that.Y; Z -= that.Z; return *this; }

	COMPILETIMEEVAL Point3D operator*(const Point3D& that) const { return {X * that.X, Y * that.Y, Z * that.Z};}
	COMPILETIMEEVAL Point3D operator*=(const Point3D& that) { X *= that.X; Y *= that.Y; Z *= that.Z; return *this; }
	COMPILETIMEEVAL Point3D operator*(int factor) const { return {X * factor, Y * factor, Z * factor};}
	COMPILETIMEEVAL Point3D& operator*=(int factor) { X *= factor; Y *= factor; Z *= factor; return *this; }

	COMPILETIMEEVAL Point3D operator%(const Point3D& that) const { return {X / that.X, Y / that.Y, Z / that.Z};}
	COMPILETIMEEVAL Point3D operator%=(const Point3D& that) { X /= that.X; Y /= that.Y; Z /= that.Z; return *this; }
	COMPILETIMEEVAL Point3D operator%(int factor) const { return {X / factor, Y / factor, Z / factor};}
	COMPILETIMEEVAL Point3D& operator%=(int factor) { X /= factor; Y /= factor; Z /= factor; return *this; }

	COMPILETIMEEVAL Point3D operator&(const Point3D& that) const { return {X & that.X, Y & that.Y, Z & that.Z};}
	COMPILETIMEEVAL Point3D operator&=(const Point3D& that) { X &= that.X; Y &= that.Y; Z &= that.Z; return *this; }
	COMPILETIMEEVAL Point3D operator&(int factor) const { return {X & factor, Y & factor, Z & factor};}
	COMPILETIMEEVAL Point3D& operator&=(int factor) { X &= factor; Y &= factor; Z &= factor; return *this; }

	COMPILETIMEEVAL bool IsValid() const { return *this != (Point3D::Empty); }

//=============================Special cases=========================================
	COMPILETIMEEVAL double powXY() const {
		return double(X * X) + double(Y * Y);
	}

	OPTIONALINLINE double LengthXY() const {
		return Math::sqrt(this->powXY());
	}

	OPTIONALINLINE double DistanceFromXY(const Point3D& that) const{
		return (that - *this).LengthXY();
	}

	COMPILETIMEEVAL double DistanceFromSquaredXY(const Point3D& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL double pow() const {
		return double(X * X) + double(Y * Y) + double (Z * Z);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Point3D& that) const{
		return (that - *this).Length();
	}

	COMPILETIMEEVAL double DistanceFromSquared(const Point3D& that) const {
		return (that - *this).pow();
	}

public:
	int X, Y, Z;
};