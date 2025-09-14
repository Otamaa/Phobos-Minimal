#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Ext/House/Body.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>

#include "Header.h"


#include <Misc/PhobosGlobal.h>

#include <RadarEventClass.h>

ASMJIT_PATCH(0x46920B, BulletClass_Detonate, 6)
{
	enum { CheckIvanBomb = 0x469343, ConituneMindControlCheck = 0x46921F, SkipEverything = 0x469AA4, Continue = 0x0 };

	GET(BulletClass* const, pThis, ESI);
	GET_BASE(const CoordStruct* const, pCoordsDetonation, 0x8);

	auto const pWarhead = pThis->WH;
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.TryFind(pThis->WeaponType);

	auto const pTechno = pThis->Owner ? pThis->Owner : nullptr;
	auto const pOwnerHouse = pTechno ? pTechno->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;

	pWHExt->Detonate(pTechno, pOwnerHouse, pThis, *pCoordsDetonation , pThis->WeaponType ? pThis->WeaponType->Damage : 0);
	PhobosGlobal::Instance()->DetonateDamageArea = false;

	// this snapping stuff does not belong here. it should go into BulletClass::Fire
	auto coords = *pCoordsDetonation;
	auto snapped = false;

	static auto const SnapDistance = 64;
	if (pThis->Target && pThis->DistanceFrom(pThis->Target) < SnapDistance) {
		coords = pThis->Target->GetCoords();
		snapped = true;
	}

	// these effects should be applied no matter what happens to the target
	 WarheadTypeExtData::CreateIonBlast(pWarhead, coords);

	bool targetStillOnMap = true;
	if (snapped && pWeaponExt && AresWPWHExt::conductAbduction(pThis->WeaponType, pThis->Owner, pThis->Target, coords)) {
		// ..and neuter the bullet, since it's not supposed to hurt the prisoner after the abduction
		pThis->Health = 0;
		pThis->DamageMultiplier = 0;
		pThis->Limbo();
		targetStillOnMap = false;
	}

	// if the target gets abducted, there's nothing there to apply IC, EMP, etc. to
	// mind that conductAbduction() neuters the bullet, so if you wish to change
	// this check, you have to fix that as well
	if (targetStillOnMap) {

		auto const damage = pThis->WeaponType ? pThis->WeaponType->Damage : 0;
		pWHExt->applyIronCurtain(coords, pOwnerHouse, damage);
		WarheadTypeExtData::applyEMP(pWarhead, coords, pThis->Owner);
		AresAE::applyAttachedEffect(pWarhead, coords, pOwnerHouse);

		if (snapped && AresWPWHExt::applyOccupantDamage(pThis)) {
			// ..and neuter the bullet, since it's not supposed to hurt the prisoner after the abduction
			pThis->Health = 0;
			pThis->DamageMultiplier = 0;
			pThis->Limbo();
		}
	}

	if(pWHExt->PermaMC)
		return 0x469AA4u;

	if (!pWHExt->MindControl_UseTreshold)
		return 0u;

	return BulletExtData::ApplyMCAlternative(pThis) ? 0x469AA4u  : 0u;
}