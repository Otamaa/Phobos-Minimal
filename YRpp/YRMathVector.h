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

	Vector2D& operator=(const Vector2D& v)
	{
		X = v.X;
		Y = v.Y;
		return *this;
	}
	//operator overloads
	//addition
	Vector2D operator+(const Vector2D& a) const
	{
		return Vector2D{ X + a.X, Y + a.Y };
	}
	//addition
	Vector2D& operator+=(const Vector2D& a)
	{
		X += a.X;
		Y += a.Y;
		return *this;
	}
	//substraction
	Vector2D operator-(const Vector2D& a) const
	{
		return Vector2D{ X - a.X, Y - a.Y };
	}
	//substraction
	Vector2D& operator-=(const Vector2D& a)
	{
		X -= a.X;
		Y -= a.Y;
		return *this;
	}
	//negation
	Vector2D operator-() const
	{
		return Vector2D{ -X, -Y };
	}
	//equality
	bool operator==(const Vector2D& a) const
	{
		return (X == a.X && Y == a.Y);
	}
	//unequality
	inline bool operator!=(const Vector2D& a) const
	{
		return (X != a.X || Y != a.Y);
	}
	//scalar multiplication
	Vector2D operator*(double r) const
	{
		return Vector2D{ static_cast<T>(X * r), static_cast<T>(Y * r) };
	}

	//scalar multiplication
	Vector2D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		return *this;
	}

	inline T& operator[](int i) { return (&X)[i]; }
	inline const T& operator[](int i) const { return (&X)[i]; }

	inline T& At(int i) { return (&X)[i]; }
	inline const T& At(int i) const { return (&X)[i]; }

	//vector multiplication
	double operator*(const Vector2D& a) const {
		return static_cast<double>(X) * a.X + static_cast<double>(Y) * a.Y;
	}

	inline bool IsValid() const { return *this != (Vector2D<T>::Empty); }
//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	inline double pow() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Vector2D<T>& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Vector2D<T>& that) const {
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

	//operator overloads
	//addition
	Vector3D operator+(const Vector3D& a) const
	{
		return Vector3D{ X + a.X, Y + a.Y, Z + a.Z };
	}

	//addition
	Vector3D& operator+=(const Vector3D& a)
	{
		X += a.X;
		Y += a.Y;
		Z += a.Z;
		return *this;
	}

	//substraction
	Vector3D operator-(const Vector3D& a) const
	{
		return Vector3D{ X - a.X, Y - a.Y, Z - a.Z };
	}

	//substraction
	Vector3D& operator-=(const Vector3D& a)
	{
		X -= a.X;
		Y -= a.Y;
		Z -= a.Z;
		return *this;
	}

	//negation
	Vector3D operator-() const
	{
		return Vector3D{ -X, -Y, -Z };
	}

	//equality
	bool operator==(const Vector3D& a) const
	{
		return (X == a.X && Y == a.Y && Z == a.Z);
	}

	//unequality
	bool operator!=(const Vector3D& a) const
	{
		return (X != a.X || Y != a.Y || Z != a.Z);
	}

	//scalar multiplication
	Vector3D operator*(double r) const
	{
		return Vector3D{
			static_cast<T>(X * r),
			static_cast<T>(Y * r),
			static_cast<T>(Z * r) };
	}

	Vector3D operator/(T nval) const
	{ return { (X / nval), (Y / nval), (Z / nval) }; }

	//scalar multiplication
	Vector3D& operator*=(double r)
	{
		X *= r;
		Y *= r;
		Z *= r;
		return *this;
	}

	inline T& operator[](int i) { return (&X)[i]; }
	inline const T& operator[](int i) const { return (&X)[i]; }

	inline T& At(int i) { return (&X)[i]; }
	inline const T& At(int i) const { return (&X)[i]; }

	//vector multiplication
	double operator*(const Vector3D& a) const
	{
		return static_cast<double>(X * a.X)
			+ static_cast<double>(Y * a.Y)
			+ static_cast<double>(Z * a.Z);
	}

	inline bool IsValid() const { return *this != (Vector3D<T>::Empty); }
//=============================Special cases=========================================
	inline double powXY() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2);
	}

	inline double LengthXY() const {
		return std::sqrt(this->powXY());
	}

	inline double DistanceFromXY(const Vector3D<T>& that) const{
		return (that - *this).LengthXY();
	}

	inline double DistanceFromSquaredXY(const Vector3D<T>& that) const {
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

	inline double DistanceFrom(const Vector3D<T>& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Vector3D<T>& that) const {
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

	inline bool operator==(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]));
	}

	inline bool operator!=(const Vector4D &b)
	{
		Vector4D a = *this;
		return ((a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]));
	}

	inline bool IsValid() const { return *this != (Vector4D<T>::Empty); }

	inline Vector4D& operator=(const Vector4D &v)
	{
		X = v.X;
		Y = v.Y;
		Z = v.Z;
		W = v.W;
		return *this;
	}

	inline Vector4D &operator+=(const Vector4D &v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;
		W += v.W;
		return *this;
	}
	inline Vector4D &operator-=(const Vector4D &v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;
		W += v.W;
		return *this;
	}
	inline Vector4D &operator*=(float k)
	{
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}
	inline Vector4D &operator/=(float k)
	{
		k = 1.0f / k;
		X = X * k;
		Y = Y * k;
		Z = Z * k;
		W = W * k;
		return *this;
	}

	inline Vector4D operator*(float k)
	{
		return Vector4D{ (X * k), (Y * k), (Z * k), (W * k) };
	}

	inline Vector4D operator/(float k)
	{
		float ook = 1.0f / k;
		Vector4D nThis = *this;
		return Vector4D{ (nThis[0] * ook), (nThis[1] * ook), (nThis[2] * ook), (nThis[3] * ook) };
	}


	inline Vector4D operator+(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis[0] + b[0],nThis[1] + b[1],nThis[2] + b[2],nThis[3] + b[3] };
	}


	inline Vector4D operator-(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return Vector4D{ nThis[0] - b[0], nThis[1] - b[1], nThis[2] - b[2], nThis[3] - b[3] };
	}

	inline double operator*(const Vector4D &b)
	{
		Vector4D nThis = *this;
		return nThis[0] * b[0] + nThis[1] * b[1] + nThis[2] * b[2] + nThis[3] * b[3];
	}

	inline T& operator[](int i) { return (&X)[i]; }
	inline const T& operator[](int i) const { return (&X)[i]; }

	inline T& At(int i) { return (&X)[i]; }
	inline const T& At(int i) const { return (&X)[i]; }

//=============================Most cases================================================
	/*
		MagnitudeSquared = pow
	*/
	inline double pow() const {
		return (double)std::pow(X,2) + (double)std::pow(Y,2) + (double)std::pow(Z,2) + (double)std::pow(W,2);
	}

	inline double Length() const {
		return std::sqrt(this->pow());
	}

	inline double DistanceFrom(const Vector4D<T>& that) const{
		return (that - *this).Length();
	}

	inline double DistanceFromSquared(const Vector4D<T>& that) const {
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
