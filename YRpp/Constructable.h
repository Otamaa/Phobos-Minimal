#pragma once

#include <CRT.h>
#include <CoordStruct.h>
#include <AbstractClass.h>
#include <WeaponTypeClass.h>

class VoxLib;
class MotLib;
// these "Construcable" is copied from original place
// added more contructor for more usage , and to prevent virtualization of the new ctor
// that can cause broken vtable
struct ConstructableWeaponStruct
{
	WeaponTypeClass* WeaponType;
	CoordStruct       FLH;
	int               BarrelLength;
	int               BarrelThickness;
	bool              TurretLocked;

	ConstructableWeaponStruct(WeaponTypeClass* pWeapon ,CoordStruct nFLH , int nBarrelLength , int bBarrelThickness ,bool bTurrentLocked)
		: WeaponType { pWeapon }
		, FLH { nFLH }
		, BarrelLength { nBarrelLength }
		, BarrelThickness { bBarrelThickness }
		, TurretLocked { bTurrentLocked }
	{}

	~ConstructableWeaponStruct() = default;

	bool operator == (const ConstructableWeaponStruct& pWeap) const
	{
		return
			!CRT::strcmpi(WeaponType->get_ID(),pWeap.WeaponType->get_ID()) &&
			FLH == pWeap.FLH &&
			BarrelLength == pWeap.BarrelLength &&
			BarrelThickness == pWeap.BarrelThickness &&
			TurretLocked == pWeap.TurretLocked
			;
	}
};

struct ConstructableVoxelStruct
{
	VoxLib* VXL { nullptr };
	MotLib* HVA { nullptr };

	bool operator == (const ConstructableVoxelStruct& nOther) const
	{ return false; } // ?
};

struct ConstructableTurretControl
{
	int Travel { -1 };
	int CompressFrames { -1 };
	int RecoverFrames { -1 };
	int HoldFrames { -1 };

	bool operator == (const ConstructableTurretControl& nOther) const
	{
		return Travel == nOther.Travel &&
			CompressFrames == nOther.CompressFrames &&
			RecoverFrames == nOther.RecoverFrames &&
			HoldFrames == nOther.HoldFrames;
	}
};

class ObjectClass;
struct DamageGroup
{
	ObjectClass* Target { nullptr };
	int Distance { 0 };

	bool operator == (const DamageGroup& nOther) const {
		return Target == (nOther.Target) &&
			Distance == nOther.Distance;
	}

	// bool operator == (const DamageGroup*& nOther) const {
	// 	return Target == (nOther->Target) &&
	// 		Distance == nOther->Distance;
	// }
};

//static_assert(sizeof(DamageGroup) == 0x8u, "Invalid size.");
