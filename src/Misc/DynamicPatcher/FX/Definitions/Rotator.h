#pragma once
#include "FloatVectors.h"

struct Rotator
{
	static const Rotator Empty;

	Rotator(float roll, float pitch, float yaw)
		: Roll(roll), Pitch(pitch), Yaw(yaw)
	{
	}

	Rotator()
		: Roll(), Pitch(), Yaw()
	{
	}

	Rotator(FloatVector3 val)
		: Roll(val.X), Pitch(val.Y), Yaw(val.Z)
	{
	}

	operator bool() const
	{ return !((*this) == Rotator::Empty); }

	Rotator operator -()
	{ return Rotator(-X, -Y, -Z); }

	Rotator operator+(Rotator& b)
	{
		return Rotator(
			 X + b.X,
			 Y + b.Y,
			 Z + b.Z);
	}
	Rotator operator-(Rotator& b)
	{
		return Rotator(
			 X - b.X,
			 Y - b.Y,
			 Z - b.Z);
	}

	Rotator operator*(double r)
	{
		return Rotator(
			 (float)(X * r),
			 (float)(Y * r),
			 (float)(Z * r));
	}

	Rotator operator/(double r)
	{
		return Rotator(
			 (float)(X / r),
			 (float)(Y / r),
			 (float)(Z / r));
	}

	operator FloatVector3() { return FloatVector3(X, Y, Z); }

	bool operator ==(Rotator& b)
	{
		return X == b.X && Y == b.Y && Z == b.Z;
	}

	bool operator!=(Rotator& b) { return !((*this) == b); }

	float Roll;
	float Pitch;
	float Yaw;

	float X { Roll };
	float Y { Pitch };
	float Z { Yaw };
};
