#pragma once


#include <YRMath.h>
#include <CellStruct.h>
/*
		Otamaa : 18/09/2021

		Moved to separate header due to compile error
		Add More element known from Vinivera-TSpp and Tompson-IDB

*/
//obvious

#pragma warning(push)
#pragma warning(disable : 4244)
struct CoordStruct
{
	int X, Y, Z;

	/*
	CoordStruct() noexcept :
		X(0), Y(0), Z(0)
	{}

	CoordStruct(int x, int y, int z) noexcept :
		X(x), Y(y), Z(z)
	{}

	//CoordStruct(CoordStruct& nCoord) noexcept :
	//	X(nCoord.X), Y(nCoord.Y), Z(nCoord.Z)
	//{}

	CoordStruct(const CoordStruct& nCoord) noexcept :
		X(nCoord.X), Y(nCoord.Y), Z(nCoord.Z)
	{}

	CoordStruct(Point3D& nPoint3) noexcept :
		X(nPoint3.X), Y(nPoint3.Y), Z(nPoint3.Z)
	{}

	//CoordStruct(const Point3D& nPoint3) noexcept :
	//	X(nPoint3.X), Y(nPoint3.Y), Z(nPoint3.Z)
	//{}

	CoordStruct(Point2D& nPoint, int nZ) noexcept :
		X(nPoint.X), Y(nPoint.Y), Z(nZ)
	{}

	CoordStruct(CellStruct& nPoint, int nZ) noexcept :
		X(nPoint.X * 256), Y(nPoint.Y * 256), Z(nZ)
	{}
	*/
public:

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Z);
	//}

	COMPILETIMEEVAL FORCEDINLINE void Snap()
	{
		// Convert coord to cell, and back again to get the absolute position.
		const CellStruct cell = this->TocellStruct();
		CoordStruct tmp {cell.X * 256 , cell.Y * 256};

		// Snap coord to cell center.
		tmp.X += 256 / 2;
		tmp.Y += 256 / 2;
		// Restore input coord Z value.
		tmp.Z = this->Z;
		*this = tmp;
	}

	COMPILETIMEEVAL FORCEDINLINE CellStruct TocellStruct() const
	{
		return { static_cast<short>(X / 256) ,static_cast<short>(Y / 256) };
	}

	//FORCEDINLINE operator bool() const {
	//	return X || Y || Z;
	//}

	COMPILETIMEEVAL FORCEDINLINE bool IsValid() const {
		return X || Y || Z;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsEmpty() const {
		return (*this) == Empty;
	}

	COMPILETIMEEVAL CoordStruct operator+(const CoordStruct& nThat) const
	{ return { X + nThat.X, Y + nThat.Y, Z + nThat.Z }; }

	COMPILETIMEEVAL CoordStruct operator+(int nThat ) const
	{ return { X + nThat, Y + nThat, Z + nThat }; }

	COMPILETIMEEVAL CoordStruct& operator+=(int nThat)
	{
		X += nThat;
		Y += nThat;
		Z += nThat;
		return *this;
	}

	COMPILETIMEEVAL CoordStruct& operator+=(const CoordStruct& nThat)
	{
		X += nThat.X;
		Y += nThat.Y;
		Z += nThat.Z;
		return *this;
	}

	COMPILETIMEEVAL CoordStruct& operator+=(const double nThat) {
		X = static_cast<int>(X + nThat);
		Y = static_cast<int>(Y + nThat);
		Z = static_cast<int>(Z + nThat);
		return *this;
	}

	COMPILETIMEEVAL CoordStruct operator-(const CoordStruct& nThat) const
	{ return { (X - nThat.X), (Y - nThat.Y), (Z - nThat.Z) }; }

	COMPILETIMEEVAL CoordStruct operator-(int nval) const
	{ return { X - nval, Y - nval, Z - nval }; }

	COMPILETIMEEVAL CoordStruct operator/(int nval) const
	{ return { X / nval, Y / nval, Z / nval }; }

	COMPILETIMEEVAL CoordStruct operator/(double ndVal) const
	{ return { static_cast<int>(X / ndVal), static_cast<int>(Y / ndVal), static_cast<int>(Z / ndVal) };}

	COMPILETIMEEVAL CoordStruct& operator-=(const CoordStruct& nThat)
	{
		X -= nThat.X;
		Y -= nThat.Y;
		Z -= nThat.Z;
		return *this;
	}

	COMPILETIMEEVAL CoordStruct operator-() const
	{ return { -X, -Y, -Z }; }

	//CoordStruct operator*(const CoordStruct& nThat) const
	//{ return { X * nThat.X, Y * nThat.Y, Z * nThat.Z }; }

	COMPILETIMEEVAL CoordStruct& operator*=(const CoordStruct& nThat)
	{
		X *= nThat.X;
		Y *= nThat.Y;
		Z *= nThat.Z;
		return *this;
	}

	//scalar multiplication
	COMPILETIMEEVAL CoordStruct operator*(double r) const
	{ return { static_cast<int>(X * r), static_cast<int>(Y * r), static_cast<int>(Z * r) }; }

	//scalar multiplication
	COMPILETIMEEVAL CoordStruct& operator*=(double r)
	{
		X = static_cast<int>(X * r);
		Y = static_cast<int>(Y * r);
		Z = static_cast<int>(Z * r);
		return *this;
	}

	COMPILETIMEEVAL bool operator==(const CoordStruct& nThat) const
	{ return (X == nThat.X && Y == nThat.Y && Z == nThat.Z); }

	COMPILETIMEEVAL bool operator!=(const CoordStruct& nThat) const
	{ return (X != nThat.X || Y != nThat.Y || Z != nThat.Z); }

	//vector multiplication
	COMPILETIMEEVAL CoordStruct operator*(const CoordStruct& a) const
	{
		return { X * a.X , Y * a.Y , Z * a.Z };
	}

	static const CoordStruct Empty;

	//OPTIONALINLINE int& operator[](int i) { return (&X)[i]; }
	//OPTIONALINLINE const int& operator[](int i) const { return (&X)[i]; }

	COMPILETIMEEVAL FORCEDINLINE int& at(int i) { return (&X)[i]; }
	COMPILETIMEEVAL FORCEDINLINE const int& at(int i) const { return (&X)[i]; }

	//cross product
	COMPILETIMEEVAL CoordStruct CrossProduct(const CoordStruct& a) const
	{
		return {
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X };
	}

	COMPILETIMEEVAL double Multiply(const CoordStruct& a) const
	{
		return static_cast<double>(X) * a.X
			+ static_cast<double>(Y) * a.Y
			+ static_cast<double>(Z) * a.Z;
	}

//=============================Special cases=========================================
	COMPILETIMEEVAL FORCEDINLINE double powXY() const {
		return double(X) * double(X) + double(Y) * double(Y);
	}

	OPTIONALINLINE double LengthXY() const {
		return Math::sqrt(this->powXY());
	}

	OPTIONALINLINE double DistanceFromXY(const CoordStruct& that) const{
		return (that - *this).LengthXY();
	}

	COMPILETIMEEVAL FORCEDINLINE double DistanceFromSquaredXY(const CoordStruct& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL FORCEDINLINE double pow() const {
		return double(X) * double(X) + double(Y) * double(Y) + double(Z) * double(Z);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const CoordStruct& that) const{
		return (that - *this).Length();
	}

	OPTIONALINLINE double DistanceFromSquared(const CoordStruct& that) const {
		return (that - *this).pow();
	}

};

typedef CoordStruct Coordinate;
#pragma warning(pop)
