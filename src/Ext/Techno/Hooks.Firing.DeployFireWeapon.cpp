#include "Body.h"


//Author : Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

//Author : Otamaa
DEFINE_HOOK(0x746CEA, UnitClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	//GET_STACK(AbstractClass*, pTarget, 0x4);

	return pType->DeployFireWeapon == -1 ? 0x746CF3 : 0x0;
}

DEFINE_HOOK(0x52190D, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x7)
{
	GET(InfantryClass*, pThis, ESI);
	return pThis->Type->DeployFireWeapon == -1 ? 0x52194E : 0x0;
}

DEFINE_HOOK(0x73DD12, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	int weaponIndex = pThis->GetTechnoType()->DeployFireWeapon;

	if (pThis->GetFireError(pThis->Target, weaponIndex, true) == FireError::OK)
	{
		pThis->Fire(pThis->Target, weaponIndex);
		auto const pWeapon = pThis->GetWeapon(weaponIndex);

		if (pWeapon && pWeapon->WeaponType->FireOnce)
			pThis->QueueMission(Mission::Guard, true);
	}

	return 0x73DD3C;
}
