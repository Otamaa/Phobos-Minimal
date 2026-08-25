#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/SWType/NewSuperWeaponType/Firewall.h>
#include <Ext/House/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/RadSite/Body.h>

#include <Misc/AresTrajectoryHelper.h>

ASMJIT_PATCH(0x4FB257, HouseClass_UnitFromFactory_Firewall, 6)
{
	GET(BuildingClass*, B, ESI);
	GET(HouseClass*, H, EBP);
	GET_STACK(CellStruct, CenterPos, 0x4C);

	BuildingExtData::BuildLines(B, CenterPos, H);

	return 0;
}

ASMJIT_PATCH(0x6D5455, TacticalClass_DrawPlacement_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EAX);
	return BuildingTypeExtData::IsLinkable(pType) ?
		0x6D545Fu : 0x6D54A9u;
}

ASMJIT_PATCH(0x6D5A5C, TacticalClass_DrawPlacement_FireWall_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	return BuildingTypeExtData::IsLinkable(pType) ?
		0x6D5A66u : 0x6D5A75u;
}

// frame to draw
ASMJIT_PATCH(0x43EFB3, BuildingClass_GetStaticImageFrame, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->GetCurrentMission() == Mission::Construction)
		return 0x43EFC6;

	const auto FrameIdx = BuildingExtData::GetImageFrameIndex(pThis);

	if (FrameIdx < 0) {
		return 0x43EFC6;
	}

	R->EAX(FrameIdx);
	return 0x43EFC3;
}

ASMJIT_PATCH(0x5880A0, MapClass_FindFirstFirestorm, 6)
{
	//GET(MapClass* const, pThis, ECX);
	GET_STACK(CoordStruct* const, pOutBuffer, STACK_OFFS(0x0, -0x4));
	GET_STACK(CoordStruct const* const, pStart, STACK_OFFS(0x0, -0x8));
	GET_STACK(CoordStruct const* const, pEnd, STACK_OFFS(0x0, -0xC));
	GET_STACK(HouseClass const* const, pOwner, STACK_OFFS(0x0, -0x10));

	*pOutBuffer = CoordStruct::Empty;

	if (HouseExtContainer::Instance.IsAnyFirestormActive && *pStart != *pEnd)
	{
		auto const start = CellClass::Coord2Cell(*pStart);
		auto const end = CellClass::Coord2Cell(*pEnd);

		for (CellSequenceEnumerator it(start, end); it; ++it)
		{
			auto const pCell = MapClass::Instance->GetCellAt(*it);
			if (auto const pBld = pCell->GetBuilding())
			{
				if (BuildingExtData::IsActiveFirestormWall(pBld, pOwner))
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

ASMJIT_PATCH(0x483D94, CellClass_UpdatePassability, 6)
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
			TechnoClass::ClearWhoTargetingThis(pCell);

			pCell->RecalcAttributes(DWORD(-1));

			if COMPILETIMEEVAL (remove)
				map->ResetZones(cell);
			else
				map->RecalculateZones(cell);

			map->RecalculateSubZones(cell);
		}
	}
}

ASMJIT_PATCH(0x440d01, BuildingClass_Put_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	//GET(CellStruct const*, pMapCoords, EBP);
	BuildingExtData::UpdateFirewallLinks(pThis);
	auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells(pThis);

	return 0;
}

// bit more early
ASMJIT_PATCH(0x445D87, BuildingClass_Limbo_DestroyableObstacle, 0x6)
{
	GET(FakeBuildingClass* const, pThis, ESI);

	auto pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells<true>(pThis);

	return 0;
}

ASMJIT_PATCH(0x445DF4, BuildingClass_Remove_FirestormWall, 6)
{
	GET(FakeBuildingClass* const, pThis, ESI);
	BuildingExtData::UpdateFirewallLinks(pThis);
	return 0;
}

ASMJIT_PATCH(0x440378, BuildingClass_Update_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->Firestorm_Wall)
		BuildingExtData::UpdateFirewall(pThis, false);

	return 0;
}

ASMJIT_PATCH(0x51BD4C, InfantryClass_Update_BuildingBelow, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);
	enum {
		canPass = 0x51BD7D,
		checkHouseFirewallActive = 0x51BD56 ,
		cannotPass = 0x51BD68
	};

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

	if (BuildingTypeExtData::IsThisBuildingPassable(pBld, pThis))
		return canPass;

	if (pTypeExt->Firestorm_Wall)
		return checkHouseFirewallActive;

	return cannotPass;
}

ASMJIT_PATCH(0x51C4C8, InfantryClass_IsCellOccupied, 6)
{
	GET(InfantryClass*, pThis , EBP);
	GET(BuildingClass* const, pBld, ESI);

	enum {

		Impassable = 0x51C7D0,
		Ignore = 0x51C70F,
		NoDecision = 0x51C4EB,
		CheckFirestorm = 0x51C4D2
	};

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

	if (BuildingTypeExtData::IsThisBuildingPassable(pBld, pThis))
		return Ignore;

	if (pTypeExt->Firestorm_Wall)
		return CheckFirestorm;

	return NoDecision;
}

ASMJIT_PATCH(0x73F7B0, UnitClass_IsCellOccupied, 6)
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

	if (BuildingTypeExtData::IsThisBuildingPassable(pBld, pThis))
		return Ignore;

	if (pTypeExt->Firestorm_Wall)
		return CheckFirestormActive;

	return NoDecision;
}

#include <Ext/Cell/Body.h>
#include <SpawnManagerClass.h>

ASMJIT_PATCH(0x4688A9, BulletClass_Unlimbo_Obstacles, 6)
{
	enum { SkipGameCode = 0x468A3F, Continue = 0x4688BD };

	GET(BulletClass* const, pThis, EBX);
	GET(CoordStruct const* const, pLocation, EDI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x54, 0x10));

	auto const pBulletOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
	if (pThis->HasParachute) {
		pThis->Velocity = VelocityClass::Empty;
		return SkipGameCode;
	}

	if (pThis->Type->Inviso) {
		if (const auto pObstacleCell = PhobosBulletObstacleHelper
			::FindFirstObstacle(*pLocation, dest, pThis->Owner, pThis->Target, pBulletOwner, pThis->Type, false, false)) {
			pThis->SetLocation(pObstacleCell->GetCoords());
			pThis->Speed = 0;
			pThis->Velocity = {};
		}else{
			// code must use pLocation because it has FlakScatter applied
			auto crdFirestorm = MapClass::Instance->FindFirstFirestorm(
				*pLocation, dest, pBulletOwner);

			if (crdFirestorm != CoordStruct::Empty) {
				crdFirestorm.Z = MapClass::Instance->GetCellFloorHeight(crdFirestorm);
				pThis->SetLocation(crdFirestorm);

				auto const pCell = MapClass::Instance->GetCellAt(crdFirestorm);
				auto const pBld = pCell->GetBuilding();
				BuildingExtData::ImmolateVictim(pBld, pThis, false);
				BulletExtData::HandleBulletRemove(pThis, ScenarioClass::Instance->Random.RandomBool(), true);

			} else {
				auto const pCell = AresBulletObstacleHelper::FindFirstObstacle(
					*pLocation, dest, pThis->Owner, pThis->Target, pThis->Type, pBulletOwner);

				pThis->SetLocation(pCell ? pCell->GetCoords() : dest);
				pThis->Speed = 0;
				pThis->Velocity = {};
			}
		}
	}

	return SkipGameCode;
}

