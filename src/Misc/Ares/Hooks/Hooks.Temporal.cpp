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

ASMJIT_PATCH(0x71AAAC, TemporalClass_Update_Abductor, 6)
{
	GET(TemporalClass*, pThis, ESI);

	const auto pOwner = pThis->Owner;
	const auto nWeaponIDx = TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp;
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	return pWeaponExt->Abductor_Temporal && AresWPWHExt::conductAbduction(pWeapon, pOwner, pThis->Target , CoordStruct::Empty)
		? 0x71AAD5 : 0x0;
}

// issue #1437: crash when warping out buildings infantry wants to garrison
ASMJIT_PATCH(0x71AA52, TemporalClass_Update_AnnounceInvalidPointer, 0x8)
{
	GET(TechnoClass*, pVictim, ECX);
	pVictim->IsAlive = false;
	return 0;
}

// issue 472: deglob WarpAway
ASMJIT_PATCH(0x71A8BD, TemporalClass_Update_WarpAway, 5)
{
	GET(TemporalClass*, pThis, ESI);

	//inside `pTarget` check
	const auto nWeaponIDx = TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp;
	auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;

	const auto pTarget = pThis->Target;

	if(auto pAnimType = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead)->Temporal_WarpAway.Get(RulesClass::Instance()->WarpAway)) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pTarget->Location ,0,1, AnimFlag(0x600),0,0),
			pThis->Owner ? pThis->Owner->Owner : nullptr,
			pTarget->Owner,
			pThis->Owner,
			false
		);
	}

	return 0x71A90E;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
// TODO :add DP stuffs here
ASMJIT_PATCH(0x71A917, TemporalClass_Update_Erase, 5)
{
	GET(TemporalClass*, pThis, ESI);

	auto pOwner = pThis->Owner;
	auto const pWeapon = pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp)->WeaponType;
	auto pOwnerExt = TechnoExtContainer::Instance.Find(pOwner);
	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (pWarheadExt->Supress_LostEva)
		pOwnerExt->SupressEVALost = true;

	if (pThis->Target && pThis->Target->IsSelected)
		pThis->Target->Deselect();

	return 0x71A97D;
}

ASMJIT_PATCH(0x71AB10, TemporalClass_GetWarpPerStep, 6)
{
	GET_STACK(int, nStep, 0x4);
	GET(TemporalClass*, pThis, ECX);
	R->EAX(TechnoExt_ExtData::GetWarpPerStep(pThis, nStep));
	return 0x71AB57;
}

// bugfix #874 A: Temporal warheads affect Warpable=no units
// skip freeing captured and destroying spawned units,
// as it is not clear here if this is warpable at all.
//TemporalClass_Fire_UnwarpableA
DEFINE_JUMP(LJMP, 0x71AF2B, 0x71AF4D);

ASMJIT_PATCH(0x71AC50, TemporalClass_LetItGo_ExpireEffect, 0x5)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		pTarget->Mark(MarkType::Redraw);

		auto nTotal = pThis->GetWarpPerStep();
		if (nTotal)
		{
			auto const pWeapon = pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp)->WeaponType;

			if (auto const Warhead = pWeapon->Warhead)
			{
				auto const pTempOwner = pThis->Owner;
				auto const peWHext = WarheadTypeExtContainer::Instance.Find(Warhead);

				if (auto pExpireAnim = peWHext->TemporalExpiredAnim.Get())
				{
					auto nCoord = pTarget->GetCenterCoords();

					{
						auto const pAnim = GameCreate<AnimClass>(pExpireAnim, nCoord);
						pAnim->ZAdjust = pTarget->GetZAdjustment() - 3;
						AnimExtData::SetAnimOwnerHouseKind(pAnim, pTempOwner->GetOwningHouse()
							, pTarget->GetOwningHouse(), pThis->Owner, false);
					}
				}

				if (peWHext->TemporalExpiredApplyDamage.Get())
				{
					auto const pTargetStreght = pTarget->GetTechnoType()->Strength;

					if (pThis->WarpRemaining > 0)
					{
						auto damage = int((pTargetStreght * ((1.0 - pThis->WarpRemaining / 10.0 / pTargetStreght)
							* (pWeapon->Damage * peWHext->TemporalDetachDamageFactor.Get()) / 100)));

						if (pTarget->IsAlive && !pTarget->IsSinking && !pTarget->IsCrashing)
							pTarget->ReceiveDamage(&damage, pTempOwner->DistanceFrom(pTarget), Warhead, pTempOwner, false, ScenarioClass::Instance->Random.RandomBool(), pTempOwner->Owner);
					}
				}
			}
		}
	}

	return 0x71AC5D;
}

ASMJIT_PATCH(0x71AFB2, TemporalClass_Fire_HealthFactor, 5)
{
	GET(TechnoClass*, pTarget, ECX);
	GET(TemporalClass*, pThis, ESI);
	GET(int, nStreght, EAX);

	auto const pWeapon = pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp)->WeaponType;;
	const auto pWarhead = pWeapon->Warhead;
	const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	const auto nCalc = (int)(((1.0 - pTarget->Health) / pTarget->GetTechnoType()->Strength) * pWarheadExt->Temporal_HealthFactor.Get());
	const double nCalc_b = (1.0 - nCalc) * (10 * nStreght) + nCalc * 0.0;

	R->EAX(nCalc_b <= 1.0 ? 1 : int(nCalc_b));
	return 0x71AFB7;
}

ASMJIT_PATCH(0x71AE50, TemporalClass_CanWarpTarget, 8)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);
	R->EAX(TechnoExt_ExtData::Warpable(pTarget));
	return 0x71AF19;
}

ASMJIT_PATCH(0x71AFD0 , TemporalClass_Logic_Unit_OreMinerUnderAttack, 0x5)
{
	GET(TemporalClass* , pThis ,ESI);

	if(auto pTarget = (UnitClass*)pThis->Target) {
		if(pTarget->Type->Harvester) {
			const auto nWeaponIDx = TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp;
			auto const pWeapon = pThis->Owner->GetWeapon(nWeaponIDx)->WeaponType;
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

			if(!pWHExt->Malicious && pTarget->Owner == HouseClass::CurrentPlayer) {
				auto nDest = pTarget->GetDestination();
				if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(nDest))) {
					VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack(), -1, -1);
				}
			}
		}
	}

	return 0x071B064;
}

ASMJIT_PATCH(0x71B0AE, TemporalClass_Logic_Building_UnderAttack, 0x7)
{
	GET(TemporalClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	BuildingExtContainer::Instance.Find(pBld)->ReceiveDamageWarhead =
		pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp)
			 ->WeaponType->Warhead;

	return 0x0;
}