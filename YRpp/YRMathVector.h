//pd's Vector classes (math sense)
#pragma once

#include <YRMath.h>
#include <tuple>

/*==========================================
============ 2D Vector =====================
==========================================*/

template <typename T> class Vector2D
{
public:
	static const Vector2D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X, Y;

	//Vector2D& operator=(const Vector2D& v)
	//{
	//	X = v.X;
	//	Y = v.Y;
	//	return *this;
	//}

	//operator overloads
	//addition
	constexpr Vector2D operator+(const Vector2D& a) const
	{
		return Vector2D{ X + a.X, Y + a.Y };
	}

	constexpr FORCEINLINE bool IsEmpty() const
	{
		return (*this) == Empty;
	}

	//addition
	constexpr Vector2D& operator+=(const Vector2D& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}
	//substraction
	constexpr Vector2D operator-(const Vector2D& a) const
	{
		return Vector2D{ X - a.X, Y - a.Y };
	}
	//substraction
	constexpr Vector2D& operator-=(const Vector2D& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}
	//negation
	constexpr Vector2D operator-() const
	{
		return Vector2D{ -X, -Y };
	}
	//equality
	constexpr bool operator==(const Vector2D& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	constexpr bool operator!=(const Vector2D& a) const
	{
		return (X != a.X || Y != a.Y);
	}
	//scalar multiplication
	constexpr Vector2D operator*(double r) const
	{
		return Vector2D{ static_cast<T>(X * r), static_cast<T>(Y * r) };
	}

	//scalar multiplication
	constexpr Vector2D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		return *this;
	}

	//inline T& operator[](int i) { return (&X)[i]; }
	//inline const T& operator[](int i) const { return (&X)[i]; }

	constexpr  T& at(int i) { return (&X)[i]; }
	constexpr  const T& at(int i) const { return (&X)[i]; }

	//vector multiplication
	constexpr double operator*(const Vector2D& a) const {
		return static_cast<double>(X) * a.X + static_cast<double>(Y) * a.Y;
	}

	constexpr   bool IsValid() const { return *this != (Vector2D<T>::Empty); }
//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr   double pow() const {
		return (double)(X * X) + (double)(Y * Y);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Vector2D<T>& that) const{
		return (*this - that).Length();
	}

	constexpr   double DistanceFromSquared(const Vector2D<T>& that) const {
		return (that - *this).pow();
	}

};

template <typename T>
const Vector2D<T> Vector2D<T>::Empty = {T(), T()};

/*==========================================
============ 3D Vector =====================
==========================================*/

template <typename T> class Vector3D
{
public:
	static const Vector3D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X,Y,Z;

	constexpr FORCEINLINE bool IsEmpty() const
	{
		return (*this) != Empty;
	}

	//operator overloads
	//addition
	constexpr Vector3D operator+(const Vector3D& a) const
	{
		return Vector3D{ X + a.X, Y + a.Y, Z + a.Z };
	}

	//addition
	constexpr Vector3D& operator+=(const Vector3D& a)
	{
		X += a.X;
		Y += a.Y;
		Z += a.Z;
		return *this;
	}

	//substraction
	constexpr Vector3D operator-(const Vector3D& a) const
	{
		return Vector3D{ X - a.X, Y - a.Y, Z - a.Z };
	}

	//substraction
	constexpr Vector3D& operator-=(const Vector3D& a)
	{
		X -= a.X;
		Y -= a.Y;
		Z -= a.Z;
		return *this;
	}

	//negation
	constexpr Vector3D operator-() const
	{
		return Vector3D{ -X, -Y, -Z };
	}

	//equality
	constexpr bool operator==(const Vector3D& a) const
	{
		return (X == a.X && Y == a.Y && Z == a.Z);
	}

	//unequality
	constexpr bool operator!=(const Vector3D& a) const
	{
		return (X != a.X || Y != a.Y || Z != a.Z);
	}

	//scalar multiplication
	constexpr Vector3D operator*(double r) const
	{
		return Vector3D{
			static_cast<T>(X * r),
			static_cast<T>(Y * r),
			static_cast<T>(Z * r) };
	}

	constexpr Vector3D operator/(T nval) const
	{ return { (X / nval), (Y / nval), (Z / nval) }; }

	//scalar multiplication
	constexpr Vector3D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		Z *= r;
		return *this;
	}

	constexpr T& operator[](int i) { return (&X)[i]; }
	constexpr const T& operator[](int i) const { return (&X)[i]; }

	constexpr  T& at(int i) { return (&X)[i]; }
	constexpr  const T& at(int i) const { return (&X)[i]; }

	//vector multiplication
	constexpr double operator*(const Vector3D& a) const
	{
		return static_cast<double>(X * a.X)
			+ static_cast<double>(Y * a.Y)
			+ static_cast<double>(Z * a.Z);
	}

	constexpr bool IsValid() const { return *this != (Vector3D<T>::Empty); }

	constexpr Vector3D<T> CrossProduct(const Vector3D<T>& a) const {
		return {
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X };
	}

	constexpr bool IsCollinearTo(const Vector3D<T>& a) const {
		return CrossProduct(a).pow() == 0;
	}


//=============================Special cases=========================================
	constexpr   double powXY() const {
		return double(X * X) + double(Y * Y);
	}

	inline double LengthXY() const {
		return std::sqrt(this->powXY());
	}

	inline double DistanceFromXY(const Vector3D<T>& that) const{
		return (that - *this).LengthXY();
	}

	constexpr   double DistanceFromSquaredXY(const Vector3D<T>& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr   double pow() const {
		return double(X * X) + double(Y * Y) + double(Z * Z);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Vector3D<T>& that) const{
		return (that - *this).Length();
	}

	constexpr   double DistanceFromSquared(const Vector3D<T>& that) const {
		return (that - *this).pow();
	}

};

template <typename T>
const Vector3D<T> Vector3D<T>::Empty = { T(), T(), T() };

template <typename T> class Vector4D
{
public:
	static const Vector4D Empty;
	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X, Y, Z, W;

	constexpr bool operator==(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] == b.at(0)) && (a[1] == b.at(1)) && (a[2] == b.at(2)) && (a[3] == b.at(3)));
	}

	constexpr bool operator!=(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] != b.at(0)) || (a[1] != b.at(1)) || (a[2] != b.at(2)) || (a[3] != b.at(3)));
	}

	constexpr  bool IsValid() const { return *this != (Vector4D<T>::Empty); }

	//inline Vector4D& operator=(const Vector4D &v)
	//{
	//	X = v.X;
	//	Y = v.Y;
	//	Z = v.Z;
	//	W = v.W;
	//	return *this;
	//}

	constexpr Vector4D &operator+=(const Vector4D &v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		W += v.W;
		return *this;
	}
	constexpr Vector4D &operator-=(const Vector4D &v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		W += v.W;
		return *this;
	}
	constexpr Vector4D &operator*=(float k)
	{
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}
	constexpr   Vector4D &operator/=(float k)
	{
		k = 1.0f / k;
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}

	constexpr Vector4D operator*(float k)
	{
		return Vector4D{ (X * k), (Y * k), (Z * k), (W * k) };
	}

	constexpr Vector4D operator/(float k)
	{
		float ook = 1.0f / k;
		Vector4D nThis = *this;
		return Vector4D{ (nThis.at(0) * ook), (nThis.at(1) * ook), (nThis.at(2) * ook), (nThis.at(3) * ook) };
	}


	constexpr Vector4D operator+(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis.at(0) + b.at(0),nThis.at(1) + b.at(1),nThis.at(2) + b.at(2),nThis.at(3) + b.at(3) };
	}


	constexpr Vector4D operator-(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis.at(0) - b.at(0), nThis.at(1) - b.at(1), nThis.at(2) - b.at(2), nThis.at(3) - b.at(3) };
	}

	constexpr double operator*(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return nThis.at(0) * b.at(0) + nThis.at(1) * b.at(1) + nThis.at(2) * b.at(2) + nThis.at(3) * b.at(3);
	}

	constexpr T& operator[](int i) { return (&X)[i]; }
	constexpr const T& operator[](int i) const { return (&X)[i]; }

	constexpr  T& at(int i) { return (&X)[i]; }
	constexpr  const T& at(int i) const { return (&X)[i]; }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	constexpr   double pow() const {
		return double(X * X) + double(Y * Y) + double(Z * Z) + double(W * W);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Vector4D<T>& that) const{
		return (that - *this).Length();
	}

	constexpr   double DistanceFromSquared(const Vector4D<T>& that) const {
		return (that - *this).pow();
	}

};

template <typename T>
const Vector4D<T> Vector4D<T>::Empty = { T(), T(), T(), T() };


template<typename T>
class PartialVector2D : public Vector2D<T>
{
public:
	size_t ValueCount;
};

template<typename T>
class PartialVector3D : public Vector3D<T>
{
public:
	size_t ValueCount;
};

template<typename T>
class ReversePartialVector3D
{
public:
	T Z;
	T Y;
	T X;
	size_t ValueCount;
};

template<typename T>
class PartialVector4D : public Vector4D<T>
{
public:
	size_t ValueCount;
};
