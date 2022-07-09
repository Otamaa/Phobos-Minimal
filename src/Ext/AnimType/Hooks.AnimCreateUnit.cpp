// Anim-to--Unit
// Author: Otamaa

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFS(0x44, -0x4));

	if(auto pExt = TechnoExt::GetExtData(pThis))
	{
		R->ECX(R->ESI());
		pExt->ReceiveDamage = true;
		AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
		pThis->Destroy();
		return 0x737F74;
	}

	return 0x0;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	if (auto const Extension = TechnoExt::GetExtData(pThis)) {
		if(!Extension->ReceiveDamage)
			AnimTypeExt::ProcessDestroyAnims(pThis);

		return 0x73887E;
	}

	return 0x0;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt && pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();

		if (auto pCell = pThis->GetCell())
			Location = pCell->GetCoordsWithBridge();
		else
			Location.Z = Map.GetCellFloorHeight(Location);

		pThis->MarkAllOccupationBits(Location);
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	if (auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
	{
		if (auto unit = pTypeExt->CreateUnit.Get())
		{
			HouseClass* decidedOwner = (pThis->Owner)
				? pThis->Owner : HouseExt::FindCivilianSide();

			auto pCell = pThis->GetCell();
			CoordStruct location = pThis->GetCoords();

			if (pCell)
				location = pCell->GetCoordsWithBridge();
			else
				location.Z = Map.GetCellFloorHeight(location);

			pThis->UnmarkAllOccupationBits(location);

			if (pTypeExt->CreateUnit_ConsiderPathfinding)
			{
				bool allowBridges = unit->SpeedType != SpeedType::Float;

				auto const nCell = Map.NearByLocation(CellClass::Coord2Cell(location),
				unit->SpeedType, -1, unit->MovementZone, false, 1, 1, true,
				false, false, allowBridges, CellStruct::Empty, false, false);

				pCell = Map[nCell];
				location = pThis->GetCoords();

				if (pCell)
					location = pCell->GetCoordsWithBridge();
				else
					location.Z = Map.GetCellFloorHeight(location);
			}

			if (auto pTechno = static_cast<TechnoClass*>(unit->CreateObject(decidedOwner)))
			{
				bool success = false;
				if (auto const pExt = AnimExt::GetExtData(pThis))
				{
					auto aFacing = pTypeExt->CreateUnit_RandomFacing.Get()
						? ScenarioGlobal->Random.RandomRangedSpecific<unsigned short>(0, 255) : pTypeExt->CreateUnit_Facing.Get();

					short resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() && pExt->FromDeathUnit)
						? pExt->DeathUnitFacing : aFacing;

					if (pCell)
						pTechno->OnBridge = pCell->ContainsBridge();

					BuildingClass* pBuilding = pCell ? 
					pCell->GetBuilding() : Map[location]->GetBuilding();

					if (!pBuilding)
					{
						++Unsorted::IKnowWhatImDoing;
						success = pTechno->Unlimbo(location, resultingFacing);
						--Unsorted::IKnowWhatImDoing;
					}
					else
					{
						success = pTechno->Unlimbo(location, resultingFacing);
					}

					if (success)
					{

						if (!decidedOwner->IsNeutral() && !unit->Insignificant) {
							decidedOwner->RegisterGain(pTechno, false);
							decidedOwner->AddTracking(pTechno);
							decidedOwner->RecheckTechTree = 1;
						}

						if (pTechno->HasTurret() && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings.Get())
							pTechno->SecondaryFacing.set(pExt->DeathUnitTurretFacing);

						pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
					}
					else
					{
						if (pTechno)
							pTechno->UnInit();
					}
				}
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}

// this set after ares set their ownership
DEFINE_HOOK(0x469C98, BulletClass_Logics_DamageAnimSelected, 0x0)
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pAnim) {
		HouseClass* pInvoker =  nullptr;
		HouseClass* pVictim = nullptr;

		if(auto pTech = pThis->Owner) {
			pInvoker =pThis->Owner->GetOwningHouse();
			if(auto const pAnimExt = AnimExt::GetExtData(pAnim))
				pAnimExt->Invoker = pTech;
		}

		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
			pVictim = Target->Owner;

		AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

	} else if (pWarheadExt && pWarheadExt->IsNukeWarhead.Get()) {
		return NukeWarheadExtras;
	}

	return Continue;
}

DEFINE_HOOK(0x6E2368, ActionClass_PlayAnimAt, 0x7)
{
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x4));

	if (pAnim) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pHouse, pHouse);
	}

	return 0;
}