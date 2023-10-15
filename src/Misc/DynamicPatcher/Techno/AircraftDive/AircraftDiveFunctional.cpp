#include "AircraftDiveFunctional.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

void AircraftDiveFunctional::Init(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt)
{
	// if (!Is_Aircraft(pExt->Get()))
	// 	return;

	// if (pTypeExt->MyDiveData.Enable) {
	// 	pExt->MyDiveData = AircraftDive(pTypeExt->MyDiveData.Speed, pTypeExt->MyDiveData.Delay);
	// }
}

void AircraftDiveFunctional::AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt)
{
	// if (!pTypeExt->MyDiveData.Enable)
	// 	return;

	// auto const pTechno = pExt->Get();
	// auto const pTarget = pTechno->Target;

	// if (!pTarget || !pTechno->IsInAir())
	// {
	// 	pExt->MyDiveData.Reset();
	// 	return;
	// }

	// CoordStruct location = pTechno->Location;
	// CoordStruct targetPos = pTarget->GetCoords();
	// targetPos.Z = MapClass::Instance->GetCellFloorHeight(targetPos);

	// int distance = pTypeExt->MyDiveData.Distance;
	// if (distance == 0) {
	// 	distance = pTechno->GetTechnoType()->GuardRange;
	// 	if (auto const pWeaponStruct = pTechno->GetWeapon(pTechno->SelectWeapon(pTarget)))
	// 		if (auto const pWeaponType = pWeaponStruct->WeaponType)
	// 			distance = pWeaponType->Range * 2;
	// }

	// if (location.DistanceFromI(targetPos) < (distance) && pExt->MyDiveData.CanDive)
	// {
	// 	int max = targetPos.Z + pTypeExt->MyDiveData.FlightLevel;
	// 	int z = location.Z - pExt->MyDiveData.Diving();
	// 	pTechno->Location.Z = z > max ? z : max;
	// }
}

void AircraftDiveFunctional::OnFire(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx)
{
	// if (!Is_Aircraft(pExt->Get()))
	// 	return;

	// if (pTypeExt->MyDiveData.PullUpAfterFire)
	// 	pExt->MyDiveData.CanDive = false;

}
