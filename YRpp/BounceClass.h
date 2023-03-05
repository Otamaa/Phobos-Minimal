/*
	Ballistic trajectory data used by Animations and Voxel Animations.
*/

#pragma once

#include <GeneralStructures.h>
#include <YRMathVector.h>
#include <Quaternion.h>

class BounceClass
{
public:
	enum class Status : int {
		None = 0,
		Bounce = 1,
		Impact = 2
	};

	// constructors
	BounceClass() = default;
	~BounceClass() = default;
	void Init(CoordStruct& coords, double elasticity, double gravity,
			double maxVelocity, Vector3D<float>* velocity, double angularVelocity) const
		{ JMP_THIS(0x4397E0); }

	void Init(CoordStruct* coords, double elasticity, int nUnused1,
		int nUnused2, double velocity, double angularVelocity) const
		{ JMP_THIS(0x439690); }

	CoordStruct* GetCoords(CoordStruct* pBuffer) const
		{ JMP_THIS(0x4399A0); }

	CoordStruct GetCoords() const {
		CoordStruct buffer;
		this->GetCoords(&buffer);
		return buffer;
	}

	double Distance() const
	  { JMP_THIS(0x439A10); }

	Matrix3D* GetDrawingMatrix(Matrix3D* pBuffer) const
		{ JMP_THIS(0x4399E0); }

	Matrix3D GetDrawingMatrix() const {
		Matrix3D buffer;
		this->GetDrawingMatrix(&buffer);
		return buffer;
	}

	Status Update()
		{ JMP_THIS(0x439B00); }

	double Elasticity{ 0.0 }; // speed multiplier when bouncing off the ground
	double Gravity{ 0.0 }; // subtracted from the Z coords every frame
	double MaxVelocity{ 0.0 }; // 0.0 disables check
	Vector3D<float> Coords {}; // position with precision
	Vector3D<float> Velocity {}; // speed components
	Quaternion CurrentAngle {}; // quaternion for drawing
	Quaternion AngularVelocity {}; // second quaternion as per-frame delta

private:
	// copy and assignment not implemented; prevent their use by declaring as private.
	BounceClass(const BounceClass&) = delete;
	BounceClass& operator=(const BounceClass&) = delete;
};

static_assert(sizeof(BounceClass) == 0x50 , "Invalid size.");