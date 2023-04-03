#include "Body.h"
#include <Ext/BulletType/Body.h>

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFS(0x3C, 0x30));
	LEA_STACK(CoordStruct*, pCoordBuffer, STACK_OFFS(0x3C, 0xC));

	if (pThis->Type->Cluster > 0)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
		{
			const int min = pTypeExt->Cluster_Scatter_Min.Get(BulletTypeExt::DefaultBulletScatterMin);
			const int max = pTypeExt->Cluster_Scatter_Max.Get(BulletTypeExt::DefaultBulletScatterMax);

			for (int i = 0; i < pThis->Type->Cluster; i++)
			{
				pThis->Detonate(origCoords);

				if (!BulletExt::IsReallyAlive(pThis))
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

	const auto nDistance = pDest->DistanceFrom(pThis->TargetCoords);
	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	const int min = pTypeExt->BallisticScatter_Min.isset() ? pTypeExt->BallisticScatter_Min : static_cast<Leptons>(0);
	const int max = pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter));
	const auto range = pThis->WeaponType ? pThis->WeaponType->Range : pThis->Range;

	R->EAX(int((nDistance * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)))
		/ range);

	return 0x4687EB;
}
