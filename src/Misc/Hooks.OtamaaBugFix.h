#pragma once

#include <Utilities/Macro.h>

#include <AnimClass.h>
#include <VoxelAnimClass.h>
#include <BounceClass.h>

class AnimClassCopy final : public AnimClass
{
public:

	AnimClass* _AnimClass_CTOR(const args_AnimClassCTOR& nArgs)
	{ JMP_THIS(0x421EA0); }
};

class VoxelAnimClassCopy final : public VoxelAnimClass
{
public:
	VoxelAnimClass* VoxelAnimClass_CTOR(const args_VoxelAnimClassCTOR& nArgs)
	{ JMP_THIS(0x7493B0); }
};

class BounceClassCopy final : public BounceClass
{
public:
	void BounceClassInit_Init(const args_BounceClassInt& nArgs)
	{ JMP_THIS(0x4397E0); }
};