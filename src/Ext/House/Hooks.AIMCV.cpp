#include "Body.h"

#include <Ext/Rules/Body.h>

ASMJIT_PATCH(0x740A11, UnitClass_Mission_Guard_AIAutoDeployMCV, 0x6)
{
	enum { SkipGameCode = 0x740A50 };

	GET(UnitClass*, pMCV, ESI);

	return (!RulesExtData::Instance()->AIAutoDeployMCV && pMCV->Owner->NumConYards > 0) ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x739889, UnitClass_TryToDeploy_AISetBaseCenter, 0x6)
{
	enum { SkipGameCode = 0x73992B };

	GET(UnitClass*, pMCV, EBP);

	return (!RulesExtData::Instance()->AISetBaseCenter && pMCV->Owner->NumConYards > 1) ? SkipGameCode : 0;
}

// AIConstructionYard Hook #4-4 -> sub_440580 - Prohibit AI from building construction yard.
ASMJIT_PATCH(0x440B7A, BuildingClass_Unlimbo_SkipSetShouldRebuild, 0x7)
{
	enum { SkipRebuildFlag = 0x440B81 };

	GET(BuildingClass* const, pBuilding, ESI);

	return (pBuilding->Type->ConstructionYard && RulesExtData::Instance()->AIForbidConYard) ? SkipRebuildFlag : 0;
}

// AIConstructionYard Hook #5-1 -> sub_588570 - Only expand walls on nodes.
ASMJIT_PATCH(0x5885D1, MapClass_BuildingToFirestormWall_SkipExtraWalls, 0x6)
{
	enum { NextDirection = 0x588730 };

	GET_STACK(const HouseClass* const, pHouse, STACK_OFFSET(0x38, 0x8));
	GET_STACK(const int, count, STACK_OFFSET(0x38, -0x24));

	if (pHouse->IsControlledByHuman() || !RulesExtData::Instance()->AINodeWallsOnly || count)
		return 0;

	GET(const CellStruct, cell, EBX);
	GET(const BuildingTypeClass* const, pType, EBP);

	const auto index = pType->ArrayIndex;
	const auto& nodes = pHouse->Base.BaseNodes;

	for (const auto& pNode : nodes) {
		if (pNode.MapCoords == cell && pNode.BuildingTypeIndex == index)
			return 0;
	}

	return NextDirection;
}

// AIConstructionYard Hook #5-2 -> sub_588750 - Only expand walls on nodes.
ASMJIT_PATCH(0x5887C1, MapClass_BuildingToWall_SkipExtraWalls, 0x6)
{
	enum { NextDirection = 0x588935 };

	GET_STACK(const HouseClass* const, pHouse, STACK_OFFSET(0x3C, 0x8));
	GET_STACK(const int, count, STACK_OFFSET(0x3C, -0x2C));

	if (pHouse->IsControlledByHuman() || !RulesExtData::Instance()->AINodeWallsOnly || count)
		return 0;

	GET(const CellStruct, cell, EDX);
	GET(const BuildingTypeClass* const, pType, EDI);

	const auto index = pType->ArrayIndex;
	const auto& nodes = pHouse->Base.BaseNodes;

	for (const auto& pNode : nodes) {
		if (pNode.MapCoords == cell && pNode.BuildingTypeIndex == index)
			return 0;
	}

	return NextDirection;
}

//ASMJIT_PATCH(0x4FD538, HouseClass_AIHouseUpdate_CheckAIBaseCenter, 0x7)
//{
//	if (RulesExtData::Instance()->AIBiasSpawnCell && !SessionClass::IsCampaign())
//	{
//		GET(HouseClass*, pAI, EBX);
//
//		if (const auto count = pAI->ConYards.Count)
//		{
//			const auto wayPoint = pAI->GetSpawnPosition();
//
//			if (wayPoint != -1)
//			{
//				const auto center = ScenarioClass::Instance->GetWaypointCoords(wayPoint);
//				auto newCenter = center;
//				double distanceSquared = 131072.0;
//
//				for (int i = 0; i < count; ++i)
//				{
//					if (const auto pBuilding = pAI->ConYards.GetItem(i))
//					{
//						if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo)
//						{
//							const auto newDistanceSquared = pBuilding->GetMapCoords().DistanceFromSquared(center);
//
//							if (newDistanceSquared < distanceSquared)
//							{
//								distanceSquared = newDistanceSquared;
//								newCenter = pBuilding->GetMapCoords();
//							}
//						}
//					}
//				}
//
//				if (newCenter != center)
//				{
//					pAI->BaseSpawnCell = newCenter;
//					pAI->Base.Center = newCenter;
//				}
//			}
//		}
//	}
//
//	return 0;
//}