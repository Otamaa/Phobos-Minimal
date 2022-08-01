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

namespace TerrainTypeTemp {
	TerrainTypeExt::ExtData* pCurrentExt = nullptr;
}

DEFINE_HOOK(0x5F4FEF, ObjectClass_Put_RegisterLogic_Terrain, 6)
{
	//GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	enum { FurtherCheck = 0x5F501B, NoUpdate = 0x5F5045 };

	if (pType->IsLogic)
	{
		if (pType->WhatAmI() == AbstractType::TerrainType)
		{
			auto const pTerrainType = static_cast<TerrainTypeClass* const>(pType);
			if (pTerrainType->SpawnsTiberium || pTerrainType->IsFlammable || pTerrainType->IsAnimated)
			{
				return FurtherCheck;
			}
		}
		else
		{
			return FurtherCheck;
		}
	}

	return NoUpdate;
}

DEFINE_HOOK(0x71B9BB, TerraiClass_TakeDamage_IsTiberiumSpawn, 0xA)
{
	enum
	{
		DoCellChailReact = 0x71BAC4,
		RetOriginalFunct = 0x0
	};

	GET(const TerrainClass*, pThis, ESI);

	auto const pTerrainTypeExt = TerrainTypeExt::ExtMap.Find(pThis->Type);

	if (!pTerrainTypeExt)
		return RetOriginalFunct;

	auto const nCoord = pThis->Location;
	auto const nDamage = pTerrainTypeExt->Damage.Get(100);
	auto const pWH = pTerrainTypeExt->Warhead.Get(RulesGlobal->C4Warhead);

	if (auto const pAnim = Map.SelectDamageAnimation(nDamage, pWH, Map[nCoord]->LandType, nCoord))
	{
		if (const auto pAnimC = GameCreate<AnimClass>(pAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, 0))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnimC, nullptr, nullptr, false);
		}
	}

	if (pTerrainTypeExt->AreaDamage)
	{
		Map.DamageArea(nCoord, nDamage, nullptr, pWH, true, nullptr);
		Map.FlashbangWarheadAt(nDamage, pWH, nCoord);
	}

	return DoCellChailReact;
}

DEFINE_HOOK(0x71C84D, TerrainClass_AI_Animated, 0x6)
{
	enum { SkipGameCode = 0x71C8D5 };

	GET(TerrainClass* const, pThis, ESI);

	if (pThis->Type) {
		if (pThis->Type->IsAnimated) {
			if (pThis->Animation.Value == pThis->Type->GetImage()->Frames / 2) {
				pThis->Animation.Value = 0;
				pThis->Animation.Start(0);

				if (pThis->Type->SpawnsTiberium && Map.IsValid(pThis->Location)) {
					if (auto const pCell = Map[pThis->Location]) {
						int cellCount = 1;
						if(auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pThis->Type)) {
							cellCount = pTypeExt->GetCellsPerAnim();

							// Set context for CellClass hooks.
							TerrainTypeTemp::pCurrentExt = pTypeExt;
						}

						for (int i = 0; i < cellCount; i++)
							pCell->SpreadTiberium(true);

						// Unset context for CellClass hooks.
						TerrainTypeTemp::pCurrentExt = nullptr;
					}
				}
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x483811, CellClass_SpreadTiberium_TiberiumType, 0x8)
{
	if (const auto pTerrainTypeExt = TerrainTypeTemp::pCurrentExt) {
		LEA_STACK(int*, pTibType, STACK_OFFS(0x1C, -0x4));
		*pTibType = pTerrainTypeExt->SpawnsTiberium_Type;
		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(0x48381D, CellClass_SpreadTiberium_CellSpread, 0x6)
{
	enum { SpreadReturn = 0x4838CA, NoSpreadReturn = 0x4838B0 };

	if (const auto pTerrainTypeExt = TerrainTypeTemp::pCurrentExt) {
		GET(CellClass*, pThis, EDI);
		GET(int, tibIndex, EAX);

		TiberiumClass* pTib = TiberiumClass::Array->GetItem(tibIndex);

		const std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(pTerrainTypeExt->SpawnsTiberium_Range);
		size_t size = adjacentCells.size();
		const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);

		for (int i = 0; i < (int)size; i++) {
			unsigned int cellIndex = (i + rand) % size;
			CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib)) {
				R->EAX<bool>(tgtCell->IncreaseTiberium(tibIndex,
					pTerrainTypeExt->GetTiberiumGrowthStage()));

				return SpreadReturn;
			}
		}

		return NoSpreadReturn;
	}

	return 0;
}

DEFINE_HOOK(0x47C065, CellClass_CellColor_TerrainRadarColor, 0x6)
{
	enum { SkipTerrainColor = 0x47C0AE, ReturnFromFunction = 0x47C0A3 };

	GET(CellClass*, pThis, ECX);
	GET_STACK(ColorStruct*, arg0, STACK_OFFS(0x14, -0x4));
	GET_STACK(ColorStruct*, arg4, STACK_OFFS(0x14, -0x8));

	if (const auto pTerrain = pThis->GetTerrain(false)) {
		if (pTerrain->Type->RadarInvisible) {
			R->ESI(pThis);
			return SkipTerrainColor;
		}
		else if (auto const pTerrainExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type)) {
			if (pTerrainExt->MinimapColor.isset())
			{
				auto& color = pTerrainExt->MinimapColor.Get();

				arg0->R = color.R;
				arg0->G = color.G;
				arg0->B = color.B;

				arg4->R = color.R;
				arg4->G = color.G;
				arg4->B = color.B;

				R->ECX(arg4);
				R->AL(color.B);

				return ReturnFromFunction;
			}
		}
	}

	return 0;
}
