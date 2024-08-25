#include "Body.h"

#include <OverlayTypeClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TerrainType/Body.h>

/* Hooks & helper functions concerning pathfinding blockages e.g enemy units on occupying cells on the way. */

class PathfindingBlockageHelper
{
public:

	static bool CanTargetObject(TechnoClass* pThis, ObjectClass* pTarget)
	{
		int primaryWeaponIndex = pThis->GetTechnoType()->TurretCount > 0 ? pThis->CurrentWeaponNumber : 0;
		auto const pWeaponPrimary = pThis->GetWeapon(primaryWeaponIndex)->WeaponType;

		if (!pWeaponPrimary || !PathfindingBlockageHelper::CanDealDamageToObject(pWeaponPrimary, pTarget) || pThis->GetFireError(pTarget, primaryWeaponIndex, true) == FireError::ILLEGAL)
		{
			auto pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;

			if (pWeaponSecondary && PathfindingBlockageHelper::CanDealDamageToObject(pWeaponSecondary, pTarget) && pThis->GetFireError(pTarget, 1, true) != FireError::ILLEGAL)
				return true;

			return false;
		}

		return true;
	}

	static bool CanDealDamageToObject(WeaponTypeClass* pThis, ObjectClass* pTarget)
	{
		if (!pThis || !pTarget)
			return false;

		auto const pExt = WeaponTypeExtContainer::Instance.Find(pThis);

		if (pExt->BlockageTargetingBypassDamageOverride.isset())
			return pExt->BlockageTargetingBypassDamageOverride.Get();

		int damage = pThis->Damage;
		auto pWarhead = pThis->Warhead;

		if (pThis->NeverUse)
			return false;

		if (damage < 1 && pThis->AmbientDamage > 0 && !pExt->AmbientDamage_IgnoreTarget)
		{
			damage = pThis->AmbientDamage;
			pWarhead = pExt->AmbientDamage_Warhead.Get(pWarhead);
		}

		double multiplier = GeneralUtils::GetWarheadVersusArmor(pWarhead, pTarget->GetType()->Armor);

		if (damage * multiplier > 0)
			return true;

		return false;
	}
};

// Hooks

DEFINE_HOOK(0x51C52D, InfantryClass_CanEnterCell_BlockageGate, 0x5)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C549, SkipToNext = 0x51C70F };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBX);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (!pThis->IsArmed())
	{
		return IsBlockage;
	}

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return SkipToNext;
}

DEFINE_HOOK(0x51C5C8, InfantryClass_CanEnterCell_BlockageGeneral1, 0x6)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C5E0 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);
	GET(AbstractType, rtti, EAX);

	// Restore overridden instructions.
	R->EDI(rtti);
	bool condition = false;

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
		condition = !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier);
	else
		condition = pThis->CombatDamage(-1) <= 0;

	if (!condition && rtti != AbstractType::Terrain)
		return IsBlockage;

	return Continue;
}

DEFINE_HOOK(0x51C841, InfantryClass_CanEnterCell_BlockageGeneral2, 0x9)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C853, Skip = 0x51C864 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBX);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (pThis->CombatDamage(-1) <= 0)
	{
		return IsBlockage;
	}

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x73F734, UnitClass_CanEnterCell_BlockageGate, 0x9)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73F73D, SkipToNext = 0x73FA87 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBP);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling && !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
		return IsBlockage;

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return SkipToNext;
}
#include <Ext/Terrain/Body.h>

DEFINE_HOOK(0x73FB71, UnitClass_CanEnterCell_BlockageGeneral1, 0x6)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73FB96 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	// Skip blockage checks for passable TerrainTypes.
	if (auto const pTerrain = abstract_cast<TerrainClass*>(pOccupier)) {
		if (TerrainExtData::CanMoveHere(pThis , pTerrain))
			return Continue;
	}

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!pThis->Type->IsTrain && !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (!pThis->Type->IsTrain && !pThis->GetWeapon(0)->WeaponType)
	{
		return IsBlockage;
	}

	return Continue;
}

DEFINE_HOOK(0x73FCA1, UnitClass_CanEnterCell_BlockageGeneral2, 0x6)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73FCE2 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	bool condition = false;

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
		condition = !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier);
	else
		condition = !pThis->GetWeapon(0)->WeaponType || !pThis->GetWeapon(0)->WeaponType->Projectile->AG;

	if (condition)
		return IsBlockage;

	return Continue;
}