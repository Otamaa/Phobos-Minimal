#include "Body.h"
#include <Ext/BulletType/Body.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	GET(FakeBulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, (0x3C - 0x30));

	if (pThis->Type->Cluster > 0)
	{
		const auto pTypeExt = pThis->_GetTypeExtData();
		const int min = pTypeExt->Cluster_Scatter_Min.Get();
		const int max = pTypeExt->Cluster_Scatter_Max.Get();
		CoordStruct coord = origCoords;

		for (int i = 0; i < pThis->Type->Cluster; i++) {

			//if (pThis->Owner) {
			//	const auto vt = VTable::Get(pThis->Owner);
			//	if (vt != AircraftClass::vtable &&
			//		vt != InfantryClass::vtable &&
			//		vt != UnitClass::vtable &&
			//		vt != BuildingClass::vtable
			//		) {
			//		pThis->Owner = nullptr;
			//	}
			//}
			pThis->Location = coord;
			pThis->Detonate(coord);
			pThis->Location = origCoords;

			if (!BulletExtData::IsReallyAlive(pThis))
				break;

			const int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
			const bool center = ScenarioClass::Instance->Random.RandomBool();
			coord = MapClass::GetRandomCoordsNear(coord, distance, center);
		}
	}

	return 0x469091;
}

int GetScatterResult(BulletClass* pThis
	, int SecondaryScatter_Proportion
	, Nullable<Leptons>& SecondaryScatter_Min
	, Nullable<Leptons>& SecondaryScatter_Max

	)
{
	const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
	int min = pTypeExt->BallisticScatterMin.Get(static_cast<Leptons>(0));
	const auto nMaxDef = Leptons(2 * RulesClass::Instance->BallisticScatter);
	int max = pTypeExt->BallisticScatterMax.Get(nMaxDef);

	if(ScenarioClass::Instance->Random.RandomFromMax(99) < SecondaryScatter_Proportion)
	{
		min = SecondaryScatter_Min.Get(static_cast<Leptons>(0));
		max = SecondaryScatter_Max.Get(nMaxDef);
	}

	return ScenarioClass::Instance->Random.RandomRanged(min , max);
}

// DEFINE_HOOK(0x4687C2 , BulletClass_MoveTo_BallisticScatter_Inviso, 6)
// {
// 	GET(BulletClass*, pThis, EBX);
// }

DEFINE_HOOK(0x46874E, BulletClass_Unlimbo_FlakScatter, 0x5)
{
	GET(FakeBulletClass*, pThis, EBX);
	GET_BASE(CoordStruct*, pDest, 0x8);

	const auto nDistance = pDest->DistanceFrom(pThis->TargetCoords);
	const auto pTypeExt = pThis->_GetTypeExtData();
	const int min = pTypeExt->BallisticScatterMin.isset() ? pTypeExt->BallisticScatterMin : static_cast<Leptons>(0);
	const int max = pTypeExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	const auto range = pThis->WeaponType ? pThis->WeaponType->Range : pThis->Range;

	R->EAX(int((nDistance * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)))
		/ range);

	return 0x4687EB;
}
