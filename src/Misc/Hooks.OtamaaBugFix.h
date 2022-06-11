#pragma once

#include <Utilities/Macro.h>

#include <AnimClass.h>
#include <VoxelAnimClass.h>
#include <BounceClass.h>

class AnimClassCopy final : public AnimClass
{
public:

	AnimClass* _AnimClass_CTOR(AnimTypeClass* pType, const CoordStruct& pCoord, int nLoopDelay,
		int nLoopCount, DWORD nflags, int nForceZAdjust, bool nReverse)
	{
		JMP_THIS(0x421EA0);
	}
};

class VoxelAnimClassCopy final : public VoxelAnimClass
{
public:
	VoxelAnimClass* VoxelAnimClass_CTOR(VoxelAnimTypeClass* pVoxelAnimType, CoordStruct* pLocation, HouseClass* pOwnerHouse)
	{ JMP_THIS(0x7493B0); }
};

class BounceClassCopy final : public BounceClass
{
public:
	void BounceClassInit_Init(CoordStruct* pCoord, double Els, double Grav, double maxVel, Vector3D<float>* pVel, double aVel)
	{ JMP_THIS(0x4397E0); }
};