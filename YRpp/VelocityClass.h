#pragma once

#include <YRMathVector.h>

struct DirStruct;
class Fixed;
// the velocities along the axes, or something like that
class VelocityClass final : public Vector3D<double>
{
public:
	static const VelocityClass Empty;

	//operator overloads
	//addition
	COMPILETIMEEVAL VelocityClass operator+(const VelocityClass& a) const {
		return { X + a.X, Y + a.Y, Z + a.Z };
	}

	//scalar multiplication
	COMPILETIMEEVAL FORCEDINLINE VelocityClass operator*(double r) const {
		return {
			X * r,
			Y * r,
			Z * r
		};
	}

	COMPILETIMEEVAL FORCEDINLINE VelocityClass operator-(const VelocityClass& a) const {
		return { X - a.X, Y - a.Y, Z - a.Z };
	}

	COMPILETIMEEVAL VelocityClass CrossProduct(const VelocityClass& a) const {
		return {
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X };
	}

	COMPILETIMEEVAL FORCEDINLINE Vector3D<double> asVec3D() const {
		return { this->X , this->Y , this->Z };
	}

	COMPILETIMEEVAL FORCEDINLINE double operator*(const VelocityClass& a) const
	{
		return static_cast<double>(X * a.X)
			+ static_cast<double>(Y * a.Y)
			+ static_cast<double>(Z * a.Z);
	}

	DirStruct* GetDirectionFromXY(DirStruct* pRetDir)
	{ JMP_THIS(0x41C2E0); }

	COMPILETIMEEVAL FORCEDINLINE void SetIfZeroXY() {
		if ( X == 0.0 && Y == 0.0 )
       		 X = 100.0;
    }

	COMPILETIMEEVAL FORCEDINLINE void SetIfZeroXYZ() {
		if ( X == 0.0 && Y == 0.0 && Z == 0.0)
       		 X = 100.0;
    }

	void Func_5B2A30(Fixed* pFixed)
	{ JMP_THIS(0x5B2A30); }
};