#ifdef COMPILE_PORTED_DP_FEATURES
#include "AircraftDiveFunctional.h"

void AircraftDiveFunctional::Init(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->Get()->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pTypeExt->MyDiveData.Enable) {
		pExt->MyDiveData = AircraftDive(pTypeExt->MyDiveData.Speed, pTypeExt->MyDiveData.Delay);
	}
}

void AircraftDiveFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->MyDiveData.Enable)
		return;

	auto const pTechno = pExt->Get();
	auto const pTarget = pTechno->Target;

	if (!pTarget || !pTechno->IsInAir())
	{
		pExt->MyDiveData.Reset();
		return;
	}

	CoordStruct location = pTechno->Location;
	CoordStruct targetPos = pTarget->GetCoords();
			    targetPos.Z = Map.GetCellFloorHeight(targetPos);

	int distance = pTypeExt->MyDiveData.Distance;
	if (distance == 0) {
		distance = pTechno->GetTechnoType()->GuardRange;
		if (auto const pWeaponStruct = pTechno->GetWeapon(pTechno->SelectWeapon(pTarget)))
			if (auto const pWeaponType = pWeaponStruct->WeaponType)
				distance = pWeaponType->Range * 2;
	}

	if (location.DistanceFromI(targetPos) < (distance) && pExt->MyDiveData.CanDive)
	{
		int max = targetPos.Z + pTypeExt->MyDiveData.FlightLevel;
		int z = location.Z - pExt->MyDiveData.Diving();
		pTechno->Location.Z = z > max ? z : max;
	}
}

void AircraftDiveFunctional::OnFire(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx)
{
	if (pExt->Get()->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pTypeExt->MyDiveData.PullUpAfterFire)
		pExt->MyDiveData.CanDive = false;

}
#endif