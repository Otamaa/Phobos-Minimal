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

	DirStruct GetDirectionFromXY()
	{
		// Compute 2D magnitude (XZ plane)
		double horizontalLength = this->LengthXY();

		// Pitch angle in radians (atan2 returns angle above/below horizontal)
		double pitchAngleRad = std::atan2(this->Z, horizontalLength);

		// Offset by -90° (engine defines 0° as vertical down)
		double adjustedPitch = pitchAngleRad - Math::DEG90_AS_RAD;

		// Convert to engine's binary angle format
		static_assert(-10430.06004058427 == Math::BINARY_ANGLE_MAGIC, "Binary Angle Magic Missmatch !");
		static_assert(1.570796326794897 == Math::DEG90_AS_RAD, "DEG90_AS_RAD Missmatch !");

		return DirStruct { static_cast<int>(static_cast<int64_t>(adjustedPitch * Math::BINARY_ANGLE_MAGIC))};
	}

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