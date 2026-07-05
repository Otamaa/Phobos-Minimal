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

	DirStruct GetDirectionFromXY();

	COMPILETIMEEVAL FORCEDINLINE void SetIfZeroXY() {
		if ( X == 0.0 && Y == 0.0 )
       		 X = 100.0;
    }

	COMPILETIMEEVAL FORCEDINLINE void SetIfZeroXYZ() {
		if ( X == 0.0 && Y == 0.0 && Z == 0.0)
       		 X = 100.0;
    }

	//Func_5B2A30
	void SetPitch (DirStruct* pFixed) {   
		// Horizontal magnitude of XY plane
		double const xyLen = Math::sqrt(this->X * this->X + this->Y * this->Y);

		// Current pitch angle in radians (angle from horizontal, converted from binary angle units)
		double const currentPitchRad =
			((Math::atan2(this->Z, xyLen) - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC - Math::BINARY_ANGLE_MASK)
			* Math::DIRECTION_FIXED_MAGIC;

		// Total 3D speed magnitude (used to recompute Z after repitch)
		double const speed3D = Math::sqrt(
			this->Z * this->Z + this->X * this->X + this->Y * this->Y);

		// Un-pitch XY: divide out current pitch cosine to recover flat horizontal components.
		// Guard: skip when currentPitchRad == 0 (already horizontal; cos(0)==1, divide is no-op).
		if (currentPitchRad != 0.0) {
			double const cosCurrentPitch = Math::cos(currentPitchRad);
			this->X /= cosCurrentPitch;
			this->Y /= cosCurrentPitch;
		}

		// New pitch angle in radians
		double const newPitchRad = (pFixed->Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;
		double const cosNewPitch = Math::cos(newPitchRad);
		double const sinNewPitch = Math::sin(newPitchRad);

		// Re-apply new pitch to XY and recompute Z from total speed
		this->X = cosNewPitch * this->X;
		this->Y = cosNewPitch * this->Y;
		this->Z = sinNewPitch * speed3D;
	}
};