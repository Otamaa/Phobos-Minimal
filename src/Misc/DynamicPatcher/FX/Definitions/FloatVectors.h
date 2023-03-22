#pragma once
#include <YRMath.h>

struct FloatVector2
{
	static const FloatVector2 Empty;
	static const FloatVector2 Unit;

	FloatVector2(float x, float y)
		: X(x), Y(y)
	{ }

	FloatVector2()
		: X(), Y()
	{ }

	bool operator==(FloatVector2& b)
	{ return X == b.X && Y == b.Y; }

	bool operator!=(FloatVector2& b) { return !((*this) == b); }

	__forceinline operator bool() const
	{ return !((*this) == FloatVector2::Empty); }

	FloatVector2 operator*(double r)
	{
		return FloatVector2(
			 (float)(X * r),
			 (float)(Y * r));
	}

	FloatVector2 operator/(double r)
	{
		return FloatVector2(
			 (float)(X / r),
			 (float)(Y / r));
	}

	FloatVector2 Normal()
	{
		return FloatVector2(-Y, X);
	}

	FloatVector2 operator-()
	{
		return FloatVector2(-X, -Y);
	}

	FloatVector2 operator+(FloatVector2& b)
	{
		return FloatVector2(
			 X + b.X,
			 Y + b.Y);
	}

	FloatVector2 operator-(FloatVector2& b)
	{
		return FloatVector2(
			 X - b.X,
			 Y - b.Y);
	}

	double operator*(FloatVector2& b)
	{
		return X * b.X + Y * b.Y;
	}

	double LengthSquared() { return (*this) * (*this); }
	double Length() { return std::sqrt(LengthSquared()); }
	FloatVector2 Direction() { return (Empty ? (*this) : ((*this) / Length())); }

	float X, Y;
};

struct FloatVector3
{
	static const FloatVector3 Empty;
	static const FloatVector3 Unit;

	FloatVector3(float x, float y, float z)
		: X(x), Y(y), Z(z)
	{ }

	FloatVector3(FloatVector2& xy, float z)
		: X(xy.X), Y(xy.Y), Z(z)
	{ }

	__forceinline operator bool() const
	{ return !((*this) == FloatVector3::Empty); }

	double LengthSquared() { return (*this) * (*this); }
	double Length() { return std::sqrt(LengthSquared()); }

	FloatVector3 Direction() { return (*this) ? (*this) : ((*this) / Length()); };

	FloatVector2 XY() { return FloatVector2(X, Y); }
	FloatVector2 XZ() { return  FloatVector2(X, Z); }
	FloatVector2 YZ() { return  FloatVector2(Y, Z); }

	FloatVector3 operator-() { return FloatVector3(-X, -Y, -Z); }

	FloatVector3 operator+(FloatVector3& b)
	{
		return FloatVector3(
			 X + b.X,
			 Y + b.Y,
			 Z + b.Z);
	}

	FloatVector3 operator-(FloatVector3& b)
	{
		return FloatVector3(
			 X - b.X,
			 Y - b.Y,
			 Z - b.Z);
	}

	FloatVector3 operator*(double r)
	{
		return FloatVector3(
			 (float)(X * r),
			 (float)(Y * r),
			 (float)(Z * r));
	}

	FloatVector3 operator/(double r)
	{
		return FloatVector3(
			 (float)(X / r),
			 (float)(Y / r),
			 (float)(Z / r));
	}

	double operator*(FloatVector3& b)
	{
		return X * b.X
			+ Y * b.Y
			+ Z * b.Z;
	}

	bool operator==(FloatVector3& b)
	{
		return X == b.X && Y == b.Y && Z == b.Z;
	}

	bool operator!=(FloatVector3& b)
	{ return!((*this) == b); }

	float X, Y, Z;
};

struct FloatVector4
{
	static const FloatVector4 Empty;
	static const FloatVector4 Unit;

	FloatVector4(float x, float y, float z, float w)
		: X(x), Y(y), Z(z) , W(w)
	{ }

	FloatVector4()
		: X(), Y(), Z(), W()
	{ }

	__forceinline operator bool() const
	{ return !((*this) == FloatVector4::Empty); }

	double LengthSquared() { return (*this) * (*this); }
	double Length() { return std::sqrt(LengthSquared()); }
	FloatVector4 Direction() { return (*this) ? (*this) : ((*this) / Length()); }

	FloatVector4 operator-()
	{
		return FloatVector4(-X, -Y, -Z, -W);
	}

	FloatVector4 operator+(FloatVector4& b)
	{
		return FloatVector4(
			 X + b.X,
			 Y + b.Y,
			 Z + b.Z,
			 W + b.W);
	}

	FloatVector4 operator-(FloatVector4& b)
	{
		return FloatVector4(
			 X - b.X,
			 Y - b.Y,
			 Z - b.Z,
			 W - b.W);
	}

	FloatVector4 operator*(double r)
	{
		return FloatVector4(
			 (float)(X * r),
			 (float)(Y * r),
			 (float)(Z * r),
			 (float)(W * r));
	}

	FloatVector4 operator/(double r)
	{
		return FloatVector4(
			 (float)(X / r),
			 (float)(Y / r),
			 (float)(Z / r),
			 (float)(W / r));
	}

	double operator*(FloatVector4& b)
	{
		return X * b.X
			+ Y * b.Y
			+ Z * b.Z
			+ W * b.W;
	}

	bool operator ==(FloatVector4& b)
	{
		return X == b.X && Y == b.Y && Z == b.Z && W == b.W;
	}

	bool operator!=(FloatVector4& b)
	{ return !((*this) == b); }

	float X , Y , Z , W;
};