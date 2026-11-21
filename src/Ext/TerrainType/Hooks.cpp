#include "Body.h"

#include <ScenarioClass.h>
#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <TerrainClass.h>
#include <SpecificStructures.h>
#include <AnimClass.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

#include <Ext/Anim/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/Rules/Body.h>


ASMJIT_PATCH(0x47C065, CellClass_CellColor_TerrainRadarColor, 0x6)
{
	enum { SkipTerrainColor = 0x47C0AE, ReturnFromFunction = 0x47C0A3 };

	GET(CellClass*, pThis, ECX);
	GET_STACK(ColorStruct*, arg0, STACK_OFFS(0x14, -0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFS(0x14, -0x8));

	if (const auto pTerrain = pThis->GetTerrain(false))
	{
		if (pTerrain->Type->RadarInvisible)
		{
			R->ESI(pThis);
			return SkipTerrainColor;
		}
		else
		{
			auto const pTerrainExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

			if (pTerrainExt->MinimapColor.isset())
			{
				const auto& color = pTerrainExt->MinimapColor;

				arg0->R = color.Fetch().R;
				arg0->G = color.Fetch().G;
				arg0->B = color.Fetch().B;

				arg4->R = color.Fetch().R;
				arg4->G = color.Fetch().G;
				arg4->B = color.Fetch().B;

				R->ECX(arg4);
				R->AL(color.Fetch().B);

				return ReturnFromFunction;
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x71C812, TerrainClass_AI_Crumbling, 0x6)
{
	enum { ReturnFromFunction = 0x71C839, SkipCheck = 0x71C7C2 };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->HasDamagedFrames && pThis->Health > 0)
	{
		if (!pThis->Type->IsAnimated && !pThis->Type->IsFlammable)
			MapClass::Logics->erase(pThis);

		pThis->TimeToDie = false;

		const auto& flammability = RulesClass::Instance->TreeFlammability;

		// burn spread probability this frame
		if (flammability > 0.0)
		{
			if (pThis->IsBurning && ScenarioClass::Instance->Random.RandomFromMax(99) == 0)
			{
				const auto pCell = pThis->GetCell();

				// check all neighbour cells that contain terrain objects and
				// roll the dice for each of them.
				for (int i = 0; i < 8; ++i)
				{
					if (auto pTree = pCell->GetNeighbourCell((FacingType)i)->GetTerrain(false))
					{
						if (!pTree->IsBurning && ScenarioClass::Instance->Random.RandomDouble() < flammability)
						{
							pTree->Ignite();
						}
					}
				}
			}
		}

		return SkipCheck;
	}

	int animationLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->CurrentHeader.Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));
	int currentStage = pThis->Animation.Stage + (pThis->Type->IsAnimated ? animationLength * (pTypeExt->HasDamagedFrames + 1) : 0 + pTypeExt->HasDamagedFrames);

	if (currentStage + 1 == pThis->Type->GetImage()->CurrentHeader.Frames / 2)
	{
		pTypeExt->PlayDestroyEffects(pThis->GetCoords());
		TerrainTypeExtData::Remove(pThis);
	}

	return ReturnFromFunction;
}

ASMJIT_PATCH(0x71C1FE, TerrainClass_Draw_PickFrame, 0x6)
{
	enum { SkipGameCode = 0x71C234 };

	GET(int, frame, EBX);

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	bool isDamaged = pTypeExt->HasDamagedFrames && pThis->GetHealthRatio() <= FakeRulesClass::Instance()->ConditionYellow_Terrain;

	if (pThis->Type->IsAnimated)
	{
		int animLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->CurrentHeader.Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));

		if (pTypeExt->HasCrumblingFrames && pThis->TimeToDie)
			frame = (animLength * (pTypeExt->HasDamagedFrames + 1)) + 1 + pThis->Animation.Stage;
		else
			frame = pThis->Animation.Stage + ((size_t)isDamaged * animLength);
	}
	else
	{
		if (pTypeExt->HasCrumblingFrames && pThis->TimeToDie)
			frame = 1 + pThis->Animation.Stage;
		else if (isDamaged)
			frame = 1;
	}

	R->EBX(frame);
	return SkipGameCode;
}

ASMJIT_PATCH(0x47EA52, CellClass_AddContent_CustomTerrainOccupation, 0x6)
{
	GET(ObjectClass*, pObject, EBP);
	if (auto const pTerrain = cast_to<TerrainClass*>(pObject); pTerrain 
		&& TerrainTypeExtData::GetCustomFoundation(pTerrain))
		return 0x47EA10; // Occupation belongs to this footprint cell rather than the terrain anchor.

	return 0;
}

ASMJIT_PATCH(0x47EB62, CellClass_RemoveContent_CustomTerrainOccupation, 0x6)
{
	GET(ObjectClass*, pObject, ESI);

	if (auto const pTerrain = cast_to<TerrainClass*>(pObject)) {
		if (auto const pExt = TerrainTypeExtData::GetCustomFoundation(pTerrain)) {
			// Passable terrain never set occupation flags, so there are none to clear.
			return pExt->IsThisPassable() ? 0x47EB8F : 0x47EB21;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x47E8CA, CellClass_AddContent_CustomTerrainTail, 0x5)
{
	enum { Append = 0x47E8D7, AlreadyLinked = 0x47EA80 };

	GET(ObjectClass*, pObject, EBP);

	if (auto const pTerrain = cast_to<TerrainClass*>(pObject); pTerrain 
		&& TerrainTypeExtData::GetCustomFoundation(pTerrain)) {
		GET(ObjectClass*, pFirst, ESI);

		for (auto pItem = pFirst; pItem; pItem = pItem->NextObject) {
			if (pItem == pTerrain)
				return AlreadyLinked;
		}

		// Buildings already use this tail insertion path. Keeping NextObject null
		// lets this terrain belong to multiple cell chains without joining them.
		return Append;
	}

	return 0;
}

//ASMJIT_PATCH(0x568841, MapClass_RemoveContentAt_Foundation_P1, 6)
//{
//	GET(BuildingClass*, pThis, EDI);
//	R->EBP(pThis->GetFoundationData(false));
//	return 0x568862;
//}

ASMJIT_PATCH(0x568862, MapClass_RemoveContentAt_TerrainFoundation, 0x8)
{
	enum { ContinueAfterCells = 0x568940 };

	GET(ObjectClass*, pObject, EDI);

	if (auto const pTerrain = cast_to<TerrainClass*>(pObject))
	{
		if (auto const pExt = TerrainTypeExtData::GetCustomFoundation(pTerrain))
		{
			GET(MapClass*, pMap, EBX);
			GET_STACK(const CellStruct*, pAnchor, 0x8B4);

			for (auto const& offset : pExt->CustomFoundation)
			{
				if (offset == TerrainTypeExtData::FoundationEnd)
					break;

				if (auto const pCell = TerrainTypeExtData::GetFoundationCell(pMap, *pAnchor, offset))
				{
					if (TerrainTypeExtData::FindCustomFoundationTerrain(pCell, pTerrain->OnBridge) != pTerrain)
						continue;

					pCell->RemoveContent(pTerrain, pTerrain->OnBridge);
					pCell->RecalcAttributes(-1);
					if (offset != CellStruct::Empty)
						TerrainTypeExtData::UpdateTerrainNeighbours(pMap, pCell->MapCoords, -1);
				}
			}

			if (!Unsorted::ScenarioInit())
			{
				// TerrainClass::Limbo resets the zones of the anchor. Reset the
				// other cells after the entire footprint has been cleared.
				for (auto const& offset : pExt->CustomFoundation)
				{
					if (offset == TerrainTypeExtData::FoundationEnd)
						break;

					if (offset == CellStruct::Empty)
						continue;

					if (auto const pCell = TerrainTypeExtData::GetFoundationCell(pMap, *pAnchor, offset))
					{
						pMap->ResetZones(pCell->MapCoords);
						pMap->RecalculateSubZones(pCell->MapCoords);
					}
				}
			}

			return ContinueAfterCells;
		}
	}

	return 0;
}


TerrainTypeExtData* TerrainTypeExtData::GetCustomFoundation(const TerrainClass* pTerrain)
{
	auto const pExt = TerrainTypeExtContainer::Instance.TryFind(pTerrain->Type);
	return pExt && pExt->HasCustomFoundation() ? pExt : nullptr;
}

CellClass* TerrainTypeExtData::GetFoundationCell(MapClass* pMap, CellStruct anchor, CellStruct offset)
{
	const CellStruct coords = anchor + offset;

	if (coords.X < 0 
		|| coords.X >= TerrainTypeExtData::MapSide
		|| coords.Y < 0 
		|| coords.Y >= TerrainTypeExtData::MapSide)
		return nullptr;

	return pMap->TryGetCellAt(coords);
}

bool TerrainTypeExtData::HasTerrainOrBuilding(const ObjectClass* pFirst)
{
	for (auto pObject = pFirst; pObject; pObject = pObject->NextObject)
	{
		const auto type = pObject->WhatAmI();
		if (type == AbstractType::Terrain || type == AbstractType::Building)
			return true;
	}

	return false;
}

void TerrainTypeExtData::UpdateTerrainNeighbours(MapClass* pMap, CellStruct coords, int delta)
{
	for (auto const& direction : CellSpread::AdjacentCell)
	{
		if (auto const pNeighbour = GetFoundationCell(pMap, coords, direction))
			pNeighbour->BlockedNeighbours += delta;
	}
}

// The base Unlimbo skips its placement test during ScenarioInit. Check the chain
// invariant even then: a custom terrain cannot share a cell with another terrain
// or building, since one NextObject link is shared by all its foundation cells.
ASMJIT_PATCH(0x71D00C, TerrainClass_Unlimbo_CustomFoundation, 0x6)
{
	enum { Fail = 0x71D13E };

	GET(TerrainClass*, pThis, ECX);
	GET(const CoordStruct*, pLocation, EBP);

	const CellStruct anchor = CellClass::Coord2Cell(*pLocation);
	const bool custom = TerrainTypeExtData::GetCustomFoundation(pThis) != nullptr;
	for (auto pOffset = pThis->GetFoundationData(false); *pOffset != TerrainTypeExtData::FoundationEnd; ++pOffset)
	{
		auto const pCell = TerrainTypeExtData::GetFoundationCell(MapClass::Instance(), anchor, *pOffset);
		if (custom)
		{
			if (!pCell 
				|| TerrainTypeExtData::HasTerrainOrBuilding(pCell->FirstObject) 
				|| TerrainTypeExtData::HasTerrainOrBuilding(pCell->AltObject))
				return Fail;
		}
		else if (pCell && (TerrainTypeExtData::FindCustomFoundationTerrain(pCell, false)
			|| TerrainTypeExtData::FindCustomFoundationTerrain(pCell, true)))
		{
			return Fail;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x568411, MapClass_AddContentAt_Foundation_P1, 6)
{
	enum { Done = 0x5687DF , SetBuildingFoundation = 0x568432};

	GET(ObjectClass*, pThis, EDI);

	// Walk the full footprint directly so custom foundations are not limited
	// by the engine's fixed-size temporary buffer.
	if (auto pTerrain = cast_to<TerrainClass*, false>(pThis))
	{
		if (auto const pExt = TerrainTypeExtData::GetCustomFoundation(pTerrain))
		{
			GET(MapClass*, pMap, EBX);
			GET_STACK(const CellStruct*, pAnchor, 0x8B4);
			for (auto const& offset : pExt->CustomFoundation)
			{
				if (offset == TerrainTypeExtData::FoundationEnd)
					break;

				if (auto const pCell = TerrainTypeExtData::GetFoundationCell(pMap, *pAnchor, offset))
				{
					pCell->AddContent(pTerrain, pTerrain->OnBridge);
					pCell->RecalcAttributes(-1);
					// TerrainClass::Unlimbo counts neighbours of the anchor itself.
					if (offset != CellStruct::Empty)
						TerrainTypeExtData::UpdateTerrainNeighbours(pMap, pCell->MapCoords, 1);
				}
			}

			return Done;
		}

		if (pTerrain->Type->Foundation == TerrainFoundation::_0x0)
			return Done;
	}

	R->EBP(pThis->GetFoundationData(false));
	return SetBuildingFoundation;
}