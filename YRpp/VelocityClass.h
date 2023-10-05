#pragma once

#include <YRMathVector.h>

struct DirStruct;
class Fixed;
// the velocities along the axes, or something like that
class VelocityClass final : public Vector3D<double>
{
public:
	//scalar multiplication
	VelocityClass operator*(double r) const {
		return {
			X * r,
			Y * r,
			Z * r
		};
	}

	VelocityClass operator-(const VelocityClass& a) const {
		return { X - a.X, Y - a.Y, Z - a.Z };
	}

	Vector3D<double> asVec3D() const {
		return { this->X , this->Y , this->Z };
	}

	DirStruct* GetDirectionFromXY(DirStruct* pRetDir)
	{ JMP_THIS(0x41C2E0); }

	void SetIfZeroXY()
	{ JMP_THIS(0x41C460); }

	void Func_5B2A30(Fixed* pFixed)
	{ JMP_THIS(0x5B2A30); }
};