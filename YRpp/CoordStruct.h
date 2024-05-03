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

	constexpr FORCEINLINE CellStruct TocellStruct()
	{
		return { static_cast<short>(X / 256) ,static_cast<short>(Y / 256) };
	}

	//FORCEINLINE operator bool() const {
	//	return X || Y || Z;
	//}

	constexpr FORCEINLINE bool IsValid() const {
		return X || Y || Z;
	}

	constexpr CoordStruct operator+(const CoordStruct& nThat) const
	{ return { X + nThat.X, Y + nThat.Y, Z + nThat.Z }; }

	constexpr CoordStruct operator+(const CoordStruct& nThat)
	{
		X += nThat.X;
		Y += nThat.Y;
		Z += nThat.Z;
		return *this;

	}

	constexpr CoordStruct operator+(int nThat ) const
	{ return { X + nThat, Y + nThat, Z + nThat }; }

	constexpr CoordStruct operator+(int nThat)
	{
		X += nThat;
		Y += nThat;
		Z += nThat;
		return *this;
	}

	constexpr CoordStruct& operator+=(const CoordStruct& nThat)
	{
		X += nThat.X;
		Y += nThat.Y;
		Z += nThat.Z;
		return *this;
	}

	constexpr CoordStruct operator+=(const double nThat) {
		return { static_cast<int>(X + nThat) ,static_cast<int>(Y + nThat) ,static_cast<int>(Z + nThat) };
	}

	constexpr CoordStruct operator-(const CoordStruct& nThat) const
	{ return { (X - nThat.X), (Y - nThat.Y), (Z - nThat.Z) }; }

	constexpr CoordStruct operator-(int nval) const
	{ return { X - nval, Y - nval, Z - nval }; }

	constexpr CoordStruct operator/(int nval) const
	{ return { X / nval, Y / nval, Z / nval }; }

	constexpr CoordStruct operator/(double ndVal) const
	{ return { static_cast<int>(X / ndVal), static_cast<int>(Y / ndVal), static_cast<int>(Z / ndVal) };}

	constexpr CoordStruct& operator-=(const CoordStruct& nThat)
	{
		X -= nThat.X;
		Y -= nThat.Y;
		Z -= nThat.Z;
		return *this;
	}

	constexpr CoordStruct operator-() const
	{ return { -X, -Y, -Z }; }

	//CoordStruct operator*(const CoordStruct& nThat) const
	//{ return { X * nThat.X, Y * nThat.Y, Z * nThat.Z }; }

	constexpr CoordStruct& operator*=(const CoordStruct& nThat)
	{
		X *= nThat.X;
		Y *= nThat.Y;
		Z *= nThat.Z;
		return *this;
	}

	//scalar multiplication
	constexpr CoordStruct operator*(double r) const
	{ return { static_cast<int>(X * r), static_cast<int>(Y * r), static_cast<int>(Z * r) }; }

	//scalar multiplication
	constexpr CoordStruct& operator*=(double r)
	{
		X = static_cast<int>(X * r);
		Y = static_cast<int>(Y * r);
		Z = static_cast<int>(Z * r);
		return *this;
	}

	constexpr bool operator==(const CoordStruct& nThat) const
	{ return (X == nThat.X && Y == nThat.Y && Z == nThat.Z); }

	constexpr bool operator!=(const CoordStruct& nThat) const
	{ return (X != nThat.X || Y != nThat.Y || Z != nThat.Z); }

	//vector multiplication
	constexpr CoordStruct operator*(const CoordStruct& a) const
	{
		return { X * a.X , Y * a.Y , Z * a.Z };
	}

	static const  CoordStruct Empty;

	//inline int& operator[](int i) { return (&X)[i]; }
	//inline const int& operator[](int i) const { return (&X)[i]; }

	constexpr FORCEINLINE int& at(int i) { return (&X)[i]; }
	constexpr FORCEINLINE const int& at(int i) const { return (&X)[i]; }

//=============================Special cases=========================================
	constexpr FORCEINLINE double powXY() const {
		return double(X * X) + double(Y * Y);
	}

	inline double LengthXY() const {
		return std::sqrt(this->powXY());
	}

	inline double DistanceFromXY(const CoordStruct& that) const{
		return (that - *this).LengthXY();
	}

	constexpr FORCEINLINE double DistanceFromSquaredXY(const CoordStruct& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr FORCEINLINE double pow() const {
		return (double)(X * X) + (double)(Y * Y) + (double)(Z * Z);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const CoordStruct& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const CoordStruct& that) const {
		return (that - *this).pow();
	}

};

typedef CoordStruct Coordinate;
#pragma warning(pop)
