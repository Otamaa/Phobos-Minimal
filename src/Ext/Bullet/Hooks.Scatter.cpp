#include "Body.h"
#include <Ext/BulletType/Body.h>

#pragma region Otamaa
//DEFINE_HOOK(0x46889D, BulletClass_Unlimbo_FlakScatter_SetTargetCoords, 0x8)
//{
//	GET(BulletClass*, pThis, EBX);
//	pThis->TargetCoords = { R->EAX<int>(),R->EDX<int>(),R->ESI<int>() };
//	return 0x0;
//}
#pragma endregion

//for fuckoff optimization !
static bool IsBulletReallyAlive(const BulletClass* const pThis)
{
	return pThis && pThis->IsAlive;
}

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(const BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFS(0x3C, 0x30));
	LEA_STACK(CoordStruct*, pCoordBuffer, STACK_OFFS(0x3C, 0xC))
		if (pThis->Type->Cluster > 0)
		{
			auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
			{
				const int min = pTypeExt->Cluster_Scatter_Min.Get(Leptons(256));
				const int max = pTypeExt->Cluster_Scatter_Max.Get(Leptons(512));

				for (int i = 0; i < pThis->Type->Cluster; i++)
				{
					pThis->Detonate(origCoords);

					// Something is deleting the bullet after this function ,..
					//
					if (!IsBulletReallyAlive(pThis))
						break;

					const int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
					auto const pNew = MapClass::GetRandomCoordsNear(pCoordBuffer, origCoords, distance, false);
					origCoords = *pNew;
				}

				return SkipGameCode;
			}
		}

	return 0x0;
}

DEFINE_HOOK(0x46874E, BulletClass_Unlimbo_FlakScatter, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_BASE(CoordStruct*, pDest, 0x8);

	auto const nDistance = pDest->DistanceFrom(pThis->TargetCoords);
	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	const int defaultmax = RulesGlobal->BallisticScatter;
	const int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
	const int max = pTypeExt->BallisticScatter_Max.Get(Leptons(defaultmax));
	auto const range = pThis->WeaponType ? pThis->WeaponType->Range : ScenarioClass::Instance->Random.RandomFromMax(255);

	R->EAX(Game::F2I((nDistance * ScenarioGlobal->Random.RandomRanged(2 * min, 2 * max)))
		/ range);

	return 0x4687EB;
}
