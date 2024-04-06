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

namespace TerrainTypeTemp
{
	TerrainTypeExtData* pCurrentExt = nullptr;
	double PriorHealthRatio = 0.0;
}

DEFINE_HOOK(0x71C84D, TerrainClass_AI_Animated, 0x6)
{
	enum { SkipGameCode = 0x71C8D5 };

	GET(TerrainClass* const, pThis, ESI);

	if (pThis->Type)
	{
		if (pThis->Type->IsAnimated)
		{
			auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

			if (pThis->Animation.Value == pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1))))
			{
				pThis->Animation.Value = 0;
				pThis->Animation.Start(0);

				if (pThis->Type->SpawnsTiberium && MapClass::Instance->IsValid(pThis->Location))
				{
					if (auto const pCell = MapClass::Instance->GetCellAt(pThis->Location))
					{
						int cellCount = 1;

						{
							cellCount = pTypeExt->GetCellsPerAnim();

							// Set context for CellClass hooks.
							TerrainTypeTemp::pCurrentExt = pTypeExt;
						}

						for (int i = 0; i < cellCount; i++)
							pCell->SpreadTiberium(true);


					}
				}

			}
			else { Debug::Log("Terrain [%s] With Corrupted Image !\n", pThis->Type->get_ID()); }
		}
	}

	// Unset context for CellClass hooks.
	TerrainTypeTemp::pCurrentExt = nullptr;
	return SkipGameCode;
}

DEFINE_HOOK(0x483811, CellClass_SpreadTiberium_TiberiumType, 0x8)
{
	//GET(CellClass*, pThis, EDI);

	if (const auto pTerrainTypeExt = TerrainTypeTemp::pCurrentExt)
	{
		LEA_STACK(int*, pTibType, STACK_OFFS(0x1C, -0x4));
		*pTibType = pTerrainTypeExt->SpawnsTiberium_Type;
		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(0x48381D, CellClass_SpreadTiberium_CellSpread, 0x6)
{
	enum { SpreadReturn = 0x4838CA, NoSpreadReturn = 0x4838B0 };

	if (const auto pTerrainTypeExt = TerrainTypeTemp::pCurrentExt)
	{
		GET(CellClass*, pThis, EDI);
		GET(int, tibIndex, EAX);

		TiberiumClass* pTib = TiberiumClass::Array->Items[tibIndex];

		std::vector<CellStruct> adjacentCells {};
		GeneralUtils::AdjacentCellsInRange(adjacentCells, pTerrainTypeExt->SpawnsTiberium_Range);
		size_t size = adjacentCells.size();
		const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);

		for (int i = 0; i < (int)size; i++)
		{
			unsigned int cellIndex = (i + rand) % size;
			CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib))
			{
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

				arg0->R = color->R;
				arg0->G = color->G;
				arg0->B = color->B;

				arg4->R = color->R;
				arg4->G = color->G;
				arg4->B = color->B;

				R->ECX(arg4);
				R->AL(color->B);

				return ReturnFromFunction;
			}
		}
	}

	return 0;
}
DEFINE_HOOK(0x71C812, TerrainClass_AI_Crumbling, 0x6)
{
	enum { ReturnFromFunction = 0x71C839, SkipCheck = 0x71C7C2 };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->HasDamagedFrames && pThis->Health > 0)
	{
		if (!pThis->Type->IsAnimated && !pThis->Type->IsFlammable)
			MapClass::Instance->Logics->Remove(pThis);

		pThis->TimeToDie = false;

		return SkipCheck;
	}

	int animationLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));
	int currentStage = pThis->Animation.Value + (pThis->Type->IsAnimated ? animationLength * (pTypeExt->HasDamagedFrames + 1) : 0 + pTypeExt->HasDamagedFrames);

	if (currentStage + 1 == pThis->Type->GetImage()->Frames / 2)
	{
		pTypeExt->PlayDestroyEffects(pThis->GetCoords());
		TerrainTypeExtData::Remove(pThis);
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x71C1FE, TerrainClass_Draw_PickFrame, 0x6)
{
	enum { SkipGameCode = 0x71C234 };

	GET(int, frame, EBX);

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	bool isDamaged = pTypeExt->HasDamagedFrames && pThis->GetHealthPercentage() <= RulesExtData::Instance()->ConditionYellow_Terrain;

	if (pThis->Type->IsAnimated)
	{
		int animLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));

		if (pTypeExt->HasCrumblingFrames && pThis->TimeToDie)
			frame = (animLength * (pTypeExt->HasDamagedFrames + 1)) + 1 + pThis->Animation.Value;
		else
			frame = pThis->Animation.Value + (isDamaged * animLength);
	}
	else
	{
		if (pTypeExt->HasCrumblingFrames && pThis->TimeToDie)
			frame = 1 + pThis->Animation.Value;
		else if (isDamaged)
			frame = 1;
	}

	R->EBX(frame);
	return SkipGameCode;
}
