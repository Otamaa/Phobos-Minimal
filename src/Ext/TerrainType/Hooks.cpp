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

	int animationLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));
	int currentStage = pThis->Animation.Stage + (pThis->Type->IsAnimated ? animationLength * (pTypeExt->HasDamagedFrames + 1) : 0 + pTypeExt->HasDamagedFrames);

	if (currentStage + 1 == pThis->Type->GetImage()->Frames / 2)
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
	bool isDamaged = pTypeExt->HasDamagedFrames && pThis->GetHealthPercentage() <= RulesExtData::Instance()->ConditionYellow_Terrain;

	if (pThis->Type->IsAnimated)
	{
		int animLength = pTypeExt->AnimationLength.Get(pThis->Type->GetImage()->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)));

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
