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

	COMPILETIMEEVAL T& operator[](int i) { return (&X)[i]; }
	COMPILETIMEEVAL const T& operator[](int i) const { return (&X)[i]; }

	//operator overloads
	//addition
	COMPILETIMEEVAL Vector2D<T> operator+(const Vector2D<T>& a) const
	{
		return { X + a.X, Y + a.Y };
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsEmpty() const
	{
		return (*this) == Empty;
	}

	//addition
	COMPILETIMEEVAL Vector2D<T>& operator+=(const Vector2D<T>& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}
	//substraction
	COMPILETIMEEVAL Vector2D<T> operator-(const Vector2D<T>& a) const
	{
		return { X - a.X, Y - a.Y };
	}
	//substraction
	COMPILETIMEEVAL Vector2D<T>& operator-=(const Vector2D<T>& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}
	//negation
	COMPILETIMEEVAL Vector2D<T> operator-() const
	{
		return { -X, -Y };
	}
	//equality
	COMPILETIMEEVAL bool operator==(const Vector2D<T>& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	COMPILETIMEEVAL bool operator!=(const Vector2D<T>&  a) const
	{
		return (X != a.X || Y != a.Y);
	}
	//scalar multiplication
	COMPILETIMEEVAL Vector2D<T> operator*(double r) const
	{
		return { static_cast<T>(X * r), static_cast<T>(Y * r) };
	}

	//scalar multiplication
	COMPILETIMEEVAL Vector2D<T>& operator*=(double r)
	{
		X *= r;
		Y *= r;
		return *this;
	}

	//OPTIONALINLINE T& operator[](int i) { return (&X)[i]; }
	//OPTIONALINLINE const T& operator[](int i) const { return (&X)[i]; }

	COMPILETIMEEVAL  T& at(int i) { return (&X)[i]; }
	COMPILETIMEEVAL  const T& at(int i) const { return (&X)[i]; }

	//vector multiplication
	COMPILETIMEEVAL double operator*(const Vector2D<T>& a) const {
		return static_cast<double>(X) * a.X + static_cast<double>(Y) * a.Y;
	}

	COMPILETIMEEVAL  bool IsValid() const { return *this != (Vector2D<T>::Empty); }

	COMPILETIMEEVAL double AngleSinTo(const Vector2D<T>& a) const
	{
		return (X * a.Y - Y * a.X) / (Length() * a.Length());
	}

	COMPILETIMEEVAL double AngleCosTo(const Vector2D<T>& a) const
	{
		return (*this * a) / (Length() * a.Length());
	}

	COMPILETIMEEVAL double FindScalar(const Vector2D<T>& a) const
	{
		double r = static_cast<double>(a.X) / static_cast<double>(X);
		if (static_cast<T>(r * Y) == a.Y)
			return r;
		else
			return std::numeric_limits<double>::quiet_NaN();
	}

	COMPILETIMEEVAL bool IsCollinearTo(const Vector2D<T>& a) const
	{
		auto const c = static_cast<double>(X) * a.Y;
		return c == static_cast<double>(Y) * a.X;
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL double pow() const {
		return double(X) * double(X) + double(Y) * double(Y);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Vector2D<T>& that) const{
		return (*this - that).Length();
	}

	COMPILETIMEEVAL double DistanceFromSquared(const Vector2D<T>& that) const {
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

	COMPILETIMEEVAL FORCEDINLINE bool IsEmpty() const
	{
		return (*this) == Empty;
	}

	//operator overloads
	//addition
	COMPILETIMEEVAL Vector3D<T> operator+(const Vector3D<T>& a) const
	{
		return { X + a.X, Y + a.Y, Z + a.Z };
	}

	//addition
	COMPILETIMEEVAL Vector3D<T>& operator+=(const Vector3D<T>& a)
	{
		X += a.X;
		Y += a.Y;
		Z += a.Z;
		return *this;
	}

	//substraction
	COMPILETIMEEVAL Vector3D<T> operator-(const Vector3D<T>& a) const
	{
		return { X - a.X, Y - a.Y, Z - a.Z };
	}

	//substraction
	COMPILETIMEEVAL Vector3D<T>& operator-=(const Vector3D<T>& a)
	{
		X -= a.X;
		Y -= a.Y;
		Z -= a.Z;
		return *this;
	}

	//negation
	COMPILETIMEEVAL Vector3D<T> operator-() const
	{
		return { -X, -Y, -Z };
	}

	//equality
	COMPILETIMEEVAL bool operator==(const Vector3D<T>& a) const
	{
		return (X == a.X && Y == a.Y && Z == a.Z);
	}

	//unequality
	COMPILETIMEEVAL bool operator!=(const Vector3D<T>& a) const
	{
		return (X != a.X || Y != a.Y || Z != a.Z);
	}

	//scalar division
	COMPILETIMEEVAL Vector3D<T> operator/(double r) const {
		if constexpr (!std::is_same_v<T, double>) {
			return { (T)(((double)X) / r) , (T)(((double)Y) / r), (T)(((double)Z) / r) };
		} else {
			return { X / r, Y / r, Z / r };
		}
	}

	//scalar division
	COMPILETIMEEVAL Vector3D<T>& operator/=(double r) {

		if constexpr (!std::is_same_v<T, double>) {
			X = (T)(((double)X) / r);
			Y = (T)(((double)Y) / r);
			Z = (T)(((double)Z) / r);
		} else {
			X /= r;
			Y /= r;
			Z /= r;
		}

		return *this;
	}

	//scalar multiplication
	COMPILETIMEEVAL Vector3D<T> operator*(double r) const
	{
		if constexpr (!std::is_same_v<T, double>) {
			return { (T)(X * r), (T)(Y * r), (T)(Z * r) };
		} else {
			return { X * r, Y * r, Z * r };
		}
	}

	//scalar multiplication
	COMPILETIMEEVAL Vector3D& operator*=(double r)
	{
		if constexpr (!std::is_same_v<T, double>){
			X = (T)(X * r);
			Y = (T)(Y * r);
			Z = (T)(Z * r);
		} else {
			X *= r;
			Y *= r;
			Z *= r;
		}
		return *this;
	}

	COMPILETIMEEVAL T& operator[](int i) { return (&X)[i]; }
	COMPILETIMEEVAL const T& operator[](int i) const { return (&X)[i]; }

	COMPILETIMEEVAL  T& at(int i) { return (&X)[i]; }
	COMPILETIMEEVAL  const T& at(int i) const { return (&X)[i]; }

	//vector multiplication
	COMPILETIMEEVAL double operator*(const Vector3D<T>& a) const
	{
		return static_cast<double>(X * a.X)
			+ static_cast<double>(Y * a.Y)
			+ static_cast<double>(Z * a.Z);
	}

	COMPILETIMEEVAL bool IsValid() const { return *this != (Vector3D<T>::Empty); }

	COMPILETIMEEVAL Vector3D<T> CrossProduct(const Vector3D<T>& a) const {
		return {
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X };
	}

	COMPILETIMEEVAL bool IsCollinearTo(const Vector3D<T>& a) const {
		return CrossProduct(a).pow() == 0;
	}

	COMPILETIMEEVAL Vector3D<T> Lerp(const Vector3D<T>& pt2, float t) {
		return (pt2 * t) + (this->operator*(double(1.0 - t)));
	}
//=============================Special cases=========================================

	COMPILETIMEEVAL   double powXY() const {
		return double(X) * double(X) + double(Y) * double(Y);
	}

	OPTIONALINLINE double LengthXY() const {
		return Math::sqrt(this->powXY());
	}

	OPTIONALINLINE double DistanceFromXY(const Vector3D<T>& that) const{
		return (that - *this).LengthXY();
	}

	COMPILETIMEEVAL   double DistanceFromSquaredXY(const Vector3D<T>& that) const {
		return (that - *this).powXY();
	}

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL double pow() const {
		return double(X) * double(X) + double(Y) * double(Y) + double(Z) * double(Z);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Vector3D<T>& that) const{
		return (that - *this).Length();
	}

	COMPILETIMEEVAL   double DistanceFromSquared(const Vector3D<T>& that) const {
		return (that - *this).pow();
	}


	//normalize
	Vector3D<T> Normalized() const
	{
		double magnitude = this->Length();
		return magnitude > 0.0 ? *this / magnitude : Vector3D<T>::Empty;
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

	COMPILETIMEEVAL bool operator==(const Vector4D<T> &b) const
	{
		Vector4D<T> a = *this;
		return ((a[0] == b.at(0)) && (a[1] == b.at(1)) && (a[2] == b.at(2)) && (a[3] == b.at(3)));
	}

	COMPILETIMEEVAL bool operator!=(const Vector4D<T>&b) const
	{
		Vector4D<T> a = *this;
		return ((a[0] != b.at(0)) || (a[1] != b.at(1)) || (a[2] != b.at(2)) || (a[3] != b.at(3)));
	}

	COMPILETIMEEVAL  bool IsValid() const { return *this != (Vector4D<T>::Empty); }

	//OPTIONALINLINE Vector4D& operator=(const Vector4D &v)
	//{
	//	X = v.X;
	//	Y = v.Y;
	//	Z = v.Z;
	//	W = v.W;
	//	return *this;
	//}

	COMPILETIMEEVAL Vector4D<T>&operator+=(const Vector4D<T>&v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		W += v.W;
		return *this;
	}
	COMPILETIMEEVAL Vector4D<T>&operator-=(const Vector4D<T>&v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		W -= v.W;
		return *this;
	}

	COMPILETIMEEVAL Vector4D<T>&operator*=(float k)
	{
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}
	COMPILETIMEEVAL Vector4D<T> &operator/=(float k)
	{
		k = 1.0f / k;
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}

	COMPILETIMEEVAL Vector4D<T> operator*(float k)
	{
		return { T(X * k), T(Y * k), T(Z * k), T(W * k) };
	}

	COMPILETIMEEVAL Vector4D<T> operator/(float k)
	{
		float ook = 1.0f / k;
		Vector4D<T> nThis = *this;
		return { T(nThis.at(0) * ook), T(nThis.at(1) * ook), T(nThis.at(2) * ook), T(nThis.at(3) * ook) };
	}

	COMPILETIMEEVAL Vector4D<T> operator+(const Vector4D<T>& v) const
	{
		return {
			X + v.X,
			Y + v.Y,
			Z + v.Z,
			W + v.W,
		};
	}

	COMPILETIMEEVAL Vector4D<T> operator-(const Vector4D<T>& v) const
	{
		return {
			X - v.X,
			Y - v.Y,
			Z - v.Z,
			W - v.W,
		};
	}

	COMPILETIMEEVAL double operator*(const Vector4D<T>&b)
	{
		Vector4D<T> nThis = *this;
		return nThis.at(0) * b.at(0) + nThis.at(1) * b.at(1) + nThis.at(2) * b.at(2) + nThis.at(3) * b.at(3);
	}

	COMPILETIMEEVAL T& operator[](int i) { return (&X)[i]; }
	COMPILETIMEEVAL const T& operator[](int i) const { return (&X)[i]; }

	COMPILETIMEEVAL  T& at(int i) { return (&X)[i]; }
	COMPILETIMEEVAL  const T& at(int i) const { return (&X)[i]; }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	COMPILETIMEEVAL   double pow() const {
		return double(X) * double(X) + double(Y) * double(Y) + double(Z) * double(Z) + double(W) * double(W);
	}

	OPTIONALINLINE double Length() const {
		return Math::sqrt(this->pow());
	}

	OPTIONALINLINE double DistanceFrom(const Vector4D<T>& that) const {
		return (that - *this).Length();
	}

	COMPILETIMEEVAL  double DistanceFromSquared(const Vector4D<T>& that) const {
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

template <typename T> class MinMaxValue {
public :
	static const MinMaxValue Empty;

public :

	T Min;
	T Max;
};

template <typename T>
const MinMaxValue<T> MinMaxValue<T>::Empty = { T(), T() };