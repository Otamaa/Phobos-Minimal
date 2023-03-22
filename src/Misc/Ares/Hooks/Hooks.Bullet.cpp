#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

DEFINE_OVERRIDE_HOOK(0x469467, BulletClass_DetonateAt_CanTemporalTarget, 0x5)
{
	GET(TechnoClass*, Target, ECX);

	switch (Target->InWhichLayer())
	{
	case Layer::Ground:
	case Layer::Air:
	case Layer::Top:
		return 0x469475;
	}

	return 0x469AA4;
}

// #1708: this mofo was raising an event without checking whether
// there is a valid tag. this is the only faulty call of this kind.
DEFINE_OVERRIDE_HOOK(0x4692A2, BulletClass_DetonateAt_RaiseAttackedByHouse, 0x6)
{
	GET(ObjectClass*, pVictim, EDI);
	return pVictim->AttachedTag ? 0 : 0x4692BD;
}

// Overpowerer no longer just infantry
DEFINE_OVERRIDE_HOOK(0x4693B0, BulletClass_DetonateAt_Overpower, 0x6)
{
	GET(TechnoClass*, pT, ECX);
	switch (pT->WhatAmI())
	{
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Building:
		return 0x4693BC;
	default:
		return 0x469AA4;
	}
}

DEFINE_OVERRIDE_HOOK(0x4664FB, BulletClass_Initialize_Ranged, 0x6)
{
	GET(BulletClass*, pThis, ECX);
	// conservative approach for legacy-initialized bullets
	pThis->Range = std::numeric_limits<int>::max();
	return 0;
}
