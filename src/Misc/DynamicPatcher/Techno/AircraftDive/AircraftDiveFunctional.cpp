#ifdef COMPILE_PORTED_DP_FEATURES
#include "AircraftDiveFunctional.h"

void AircraftDiveFunctional::Init(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->OwnerObject()->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pTypeExt->MyDiveData.Enable) {
		pExt->MyDiveData = AircraftDive(pTypeExt->MyDiveData.Speed, pTypeExt->MyDiveData.Delay);
	}
}

void AircraftDiveFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->MyDiveData.Enable)
		return;

	auto const pTechno = pExt->OwnerObject();
	auto const pTarget = pTechno->Target;

	if (!pTarget || !pTechno->IsInAir())
	{
		pExt->MyDiveData.Reset();
		return;
	}

	CoordStruct location = pTechno->Location;
	CoordStruct targetPos = pTarget->GetCoords();

	int distance = pTypeExt->MyDiveData.Distance;
	int nBackupDistance = pTechno->GetTechnoType()->GuardRange;

	if (distance == 0)
	{
		int const weaponIndex = pTechno->SelectWeapon(pTarget);
		auto const pWeaponStruct = pTechno->GetWeapon(weaponIndex);

		if (!pWeaponStruct)
			distance = nBackupDistance;

		auto const pWeaponType = pWeaponStruct->WeaponType;

		if (!pWeaponType)
			distance = nBackupDistance;

		distance = pWeaponType->Range * 2;
	}

	if (location.DistanceFrom(targetPos) < distance && pExt->MyDiveData.CanDive)
	{
		int max = targetPos.Z + pTypeExt->MyDiveData.FlightLevel;
		int z = location.Z - pExt->MyDiveData.Diving();
		pTechno->Location.Z = z > max ? z : max;
	}
}

void AircraftDiveFunctional::OnFire(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx)
{
	if (pExt->OwnerObject()->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pTypeExt->MyDiveData.PullUpAfterFire)
		pExt->MyDiveData.CanDive = false;

}
#endif