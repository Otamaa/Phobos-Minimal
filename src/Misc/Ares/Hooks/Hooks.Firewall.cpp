#include <Ext/Rules/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/SWType/NewSuperWeaponType/Firewall.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>

#include "Header.h"
#include "AresTrajectoryHelper.h"

DEFINE_HOOK(0x4FB257, HouseClass_UnitFromFactory_Firewall, 6)
{
	GET(BuildingClass*, B, ESI);
	GET(HouseClass*, H, EBP);
	GET_STACK(CellStruct, CenterPos, 0x4C);

	FirewallFunctions::BuildLines(B, CenterPos, H);

	return 0;
}

DEFINE_HOOK(0x445355, BuildingClass_KickOutUnit_Firewall, 6)
{
	GET(BuildingClass*, Factory, ESI);
	GET(BuildingClass*, B, EDI);
	GET_STACK(CellStruct, CenterPos, 0x20);

	FirewallFunctions::BuildLines(B, CenterPos, Factory->Owner);

	return 0;
}

DEFINE_HOOK(0x6D5455, TacticalClass_DrawPlacement_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EAX);
	return BuildingTypeExtData::IsLinkable(pType) ?
		0x6D545Fu : 0x6D54A9u;
}

DEFINE_HOOK(0x6D5A5C, TacticalClass_DrawPlacement_FireWall_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	return BuildingTypeExtData::IsLinkable(pType) ?
		0x6D5A66u : 0x6D5A75u;
}

// frame to draw
DEFINE_HOOK(0x43EFB3, BuildingClass_GetStaticImageFrame, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->GetCurrentMission() == Mission::Construction)
		return 0x43EFC6;

	const auto FrameIdx = FirewallFunctions::GetImageFrameIndex(pThis);

	if (FrameIdx < 0) {
		return 0x43EFC6;
	}

	R->EAX(FrameIdx);
	return 0x43EFC3;
}

DEFINE_HOOK(0x5880A0, MapClass_FindFirstFirestorm, 6)
{
	//GET(MapClass* const, pThis, ECX);
	GET_STACK(CoordStruct* const, pOutBuffer, STACK_OFFS(0x0, -0x4));
	GET_STACK(CoordStruct const* const, pStart, STACK_OFFS(0x0, -0x8));
	GET_STACK(CoordStruct const* const, pEnd, STACK_OFFS(0x0, -0xC));
	GET_STACK(HouseClass const* const, pOwner, STACK_OFFS(0x0, -0x10));

	*pOutBuffer = CoordStruct::Empty;

	if (HouseExtData::IsAnyFirestormActive && *pStart != *pEnd)
	{
		auto const start = CellClass::Coord2Cell(*pStart);
		auto const end = CellClass::Coord2Cell(*pEnd);

		for (CellSequenceEnumerator it(start, end); it; ++it)
		{
			auto const pCell = MapClass::Instance->GetCellAt(*it);
			if (auto const pBld = pCell->GetBuilding())
			{
				if (FirewallFunctions::IsActiveFirestormWall(pBld, pOwner))
				{
					*pOutBuffer = CellClass::Cell2Coord(*it);
					break;
				}
			}
		}
	}

	R->EAX(pOutBuffer);
	return 0x58855E;
}

DEFINE_HOOK(0x483D94, CellClass_UpdatePassability, 6)
{
	GET(BuildingClass* const, pBuilding, ESI);
	return BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Firestorm_Wall ? 0x483D9E : 0x483DB0;
}

template <bool remove = false>
static void RecalculateCells(BuildingClass* pThis)
{
	auto const cells = BuildingExtData::GetFoundationCells(pThis, pThis->GetMapCoords());

	auto& map = MapClass::Instance;

	for (auto const& cell : cells)
	{
		if (auto pCell = map->TryGetCellAt(cell))
		{
			pCell->RecalcAttributes(DWORD(-1));

			if COMPILETIMEEVAL (remove)
				map->ResetZones(cell);
			else
				map->RecalculateZones(cell);

			map->RecalculateSubZones(cell);

		}
	}
}

DEFINE_HOOK(0x440d01, BuildingClass_Put_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	//GET(CellStruct const*, pMapCoords, EBP);
	FirewallFunctions::UpdateFirewallLinks(pThis);
	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells(pThis);

	return 0;
}

DEFINE_HOOK(0x445DF4, BuildingClass_Remove_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	FirewallFunctions::UpdateFirewallLinks(pThis);
	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells<true>(pThis);

	return 0;
}

DEFINE_HOOK(0x440378, BuildingClass_Update_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->Firestorm_Wall)
		FirewallFunctions::UpdateFirewall(pThis, false);

	return 0;
}

DEFINE_HOOK(0x51BD4C, InfantryClass_Update_BuildingBelow, 6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(BuildingClass*, pBld, EDI);
	enum {
		canPass = 0x51BD7D,
		checkHouseFirewallActive = 0x51BD56 ,
		cannotPass = 0x51BD68
	};

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

	if (pTypeExt->IsPassable)
		return canPass;

	if (pTypeExt->Firestorm_Wall)
		return checkHouseFirewallActive;

	return cannotPass;
}

DEFINE_HOOK(0x51C4C8, InfantryClass_IsCellOccupied, 6)
{
	GET(InfantryClass* const, pThis , EBP);
	GET(BuildingClass* const, pBld, ESI);

	enum {

		Impassable = 0x51C7D0,
		Ignore = 0x51C70F,
		NoDecision = 0x51C4EB,
		CheckFirestorm = 0x51C4D2
	};

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

	if (pTypeExt->IsPassable)
		return Ignore;

	if (pTypeExt->Firestorm_Wall)
		return CheckFirestorm;

	return NoDecision;
}

DEFINE_HOOK(0x73F7B0, UnitClass_IsCellOccupied, 6)
{
	GET(UnitClass* const , pThis , EBX);
	GET(BuildingClass* const, pBld, ESI);

	enum
	{
		Impassable = 0x73FCD0, // return Move_No
		Ignore = 0x73FA87, // check next object
		NoDecision = 0x73F7D3, // check other
		CheckFirestormActive = 0x73F7BA // check if the object owner has FirestromActive flag
	};

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

	if (pTypeExt->IsPassable)
		return Ignore;

	if (pTypeExt->Firestorm_Wall)
		return CheckFirestormActive;

	return NoDecision;
}

#include <Ext/Cell/Body.h>

DEFINE_HOOK(0x4DA54E, FootClass_Update_AresAddition, 6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->HasRemainingWarpInDelay) {
		if (pExt->LastWarpInDelay) {
			pExt->LastWarpInDelay--;
		}
		else {
			pExt->HasRemainingWarpInDelay = false;
			pExt->IsBeingChronoSphered = false;
			pThis->WarpingOut = false;
		}
	}

	if (HouseExtData::IsAnyFirestormActive) {
		if (pThis->IsAlive && !pThis->InLimbo && !pThis->InOpenToppedTransport && !pType->IgnoresFirestorm) {
			if (auto const pBld = pThis->GetCell()->GetBuilding()) {
				if (FirewallFunctions::IsActiveFirestormWall(pBld, nullptr)) {
					FirewallFunctions::ImmolateVictim(pBld, pThis, true);
				}
			}
		}
	}

	// tiberium heal, as in Tiberian Sun, but customizable per Tiberium type
	if (pThis->IsAlive && RulesExtData::Instance()->Tiberium_HealEnabled
		&& pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		if (pType->TiberiumHeal || pThis->HasAbility(AbilityType::TiberiumHeal))
		{
			if (pThis->Health > 0 && pThis->Health < pType->Strength)
			{
				bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
				auto const pCell = (FakeCellClass*)pThis->GetCell();

				if (pCell->LandType == LandType::Tiberium)
				{
					auto delay = RulesClass::Instance->TiberiumHeal;
					auto health = pType->GetRepairStep();

					int idxTib = pCell->_GetTiberiumType();
					if (auto const pTib = TiberiumClass::Array->GetItemOrDefault(idxTib))
					{
						auto pExt = TiberiumExtContainer::Instance.Find(pTib);
						delay = pExt->GetHealDelay();
						health = pExt->GetHealStep(pThis);
					}

					if (health != 0)
					{
						if (!(Unsorted::CurrentFrame % int(delay * 900.0)))
						{
							pThis->Health += health;

							if (pThis->Health > pType->Strength) {
								pThis->Health = pType->Strength;
							}

							if (wasDamaged
								&& (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
								|| pThis->GetHeight() < -10))
							{
								if (auto& dmgParticle = pThis->DamageParticleSystem)
								{
									dmgParticle->UnInit();
									dmgParticle = nullptr;
								}
							}
						}
					}
				}
			}
		}
	}

	return pThis->IsAlive ? 0 : 0x4DAF00;
}

DEFINE_HOOK(0x467B94, BulletClass_Update_Ranged, 7)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(bool, Destroy, 0x18);
	REF_STACK(CoordStruct, CrdNew, 0x24);

	// range check
	if (pThis->Type->Ranged)
	{
		CoordStruct crdOld = pThis->GetCoords();

		pThis->Range -= int(CrdNew.DistanceFrom(crdOld));
		if (pThis->Range <= 0)
		{
			Destroy = true;
		}
	}

	// replicate replaced instruction
	pThis->SetLocation(CrdNew);

	// firestorm wall check
	if (HouseExtData::IsAnyFirestormActive && !pThis->Type->IgnoresFirestorm)
	{
		auto const pCell = MapClass::Instance->GetCellAt(CrdNew);

		if (auto const pBld = pCell->GetBuilding())
		{
			HouseClass* pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
			if (WarheadTypeExtContainer::Instance.Find(RulesExtData::Instance()->FirestormWarhead)->CanAffectHouse(pBld->Owner, pOwner))
				pOwner =  nullptr; // clear the pointer if can affect the bullet owner

			if (FirewallFunctions::IsActiveFirestormWall(pBld, pOwner))
			{
				FirewallFunctions::ImmolateVictim(pBld , pThis, false);
				BulletExtData::HandleBulletRemove(pThis, ScenarioClass::Instance->Random.RandomBool(), true);
				return 0x467FBA;
			}
		}
	}

	return 0x467BA4;
}

DEFINE_HOOK(0x4688BD, BulletClass_SetMovement_Obstacle, 6)
{
	GET(BulletClass* const, pThis, EBX);
	GET(CoordStruct const* const, pLocation, EDI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x54, 0x10));

	auto const pBulletOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;

	// code must use pLocation because it has FlakScatter applied
	auto crdFirestorm = MapClass::Instance->FindFirstFirestorm(
		*pLocation, dest, pBulletOwner);

	if (crdFirestorm != CoordStruct::Empty)
	{
		crdFirestorm.Z = MapClass::Instance->GetCellFloorHeight(crdFirestorm);
		pThis->SetLocation(crdFirestorm);

		auto const pCell = MapClass::Instance->GetCellAt(crdFirestorm);
		auto const pBld = pCell->GetBuilding();
		FirewallFunctions::ImmolateVictim(pBld, pThis, false);
		BulletExtData::HandleBulletRemove(pThis, ScenarioClass::Instance->Random.RandomBool(), true);

	}
	else
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
		auto const pCell = AresTrajectoryHelper::FindFirstObstacle(
			*pLocation, dest, pThis->Owner, pThis->Target, pThis->Type,
			pTypeExt, pBulletOwner);

		pThis->SetLocation(pCell ? pCell->GetCoords() : dest);
		pThis->Speed = 0;
		pThis->Velocity = {};
	}

	return 0x468A3F;
}

DEFINE_HOOK_AGAIN(0x6FF860, TechnoClass_Fire_FSW, 8)
DEFINE_HOOK(0x6FF008, TechnoClass_Fire_FSW, 8)
{
	REF_STACK(CoordStruct const, src, 0x44);
	REF_STACK(CoordStruct const, tgt, 0x88);

	if (!HouseExtData::IsAnyFirestormActive) {
		return 0;
	}

	auto const Bullet = R->Origin() == 0x6FF860
		? R->EDI<BulletClass*>()
		: R->EBX<BulletClass*>()
		;

	if (!Bullet->Type->IgnoresFirestorm) {
		return 0;
	}

	auto const crd = MapClass::Instance->FindFirstFirestorm(src, tgt, Bullet->Owner->Owner);

	if (crd.IsValid()) {
		Bullet->Target = MapClass::Instance->GetCellAt(crd)->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
	}

	return 0;
}