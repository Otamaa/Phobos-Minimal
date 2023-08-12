#include "Body.h"
#include <Ext/TechnoType/Body.h>

//Author : Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

DEFINE_HOOK(0x52190D, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6) //7
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pThisType, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x8);

	if (pThisType->DeployFireWeapon == -1) {
		R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	} else {
		R->EAX(pThisType->DeployFireWeapon);
	}

	return 0x521913;
}

#ifdef DISABLEFORTESTINGS
DEFINE_HOOK(0x6FF925, TechnoClass_FireaAt_FireOnce, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	pThis->SetTarget(nullptr);
	if (auto pUnit = specific_cast<UnitClass*>(pThis)) {
		if (pUnit->Type->DeployFire
			&& !pUnit->Type->IsSimpleDeployer
			&& !pUnit->Deployed
			&& pThis->CurrentMission == Mission::Unload
		)
		{
			TechnoExt::ExtMap.Find(pUnit)->DeployFireTimer.Start(pWeapon->ROF);
		}
	}

	return 0x6FF92F;
}

DEFINE_HOOK(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C, SetMissionGuard = 0x73DEBA };

	GET(UnitClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->DeployFireTimer.InProgress())
	{
		auto const nWeapIdx = TechnoExt::GetDeployFireWeapon(pThis);
		auto pTarget = pThis->GetCell();
		pThis->SetTarget(pTarget);

		if (pThis->GetFireError(pTarget, nWeapIdx, true) == FireError::OK)
		{
			auto pWeapon = pThis->GetWeapon(nWeapIdx);

			pThis->Fire(pThis->GetCell(), nWeapIdx);

			if (pWeapon->WeaponType->FireOnce) {
				R->EBX(0);
				return SetMissionGuard;
			}
		}
	}
	else
	{
		pThis->SetTarget(nullptr);
		R->EBX(0);
		return SetMissionGuard;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x741288, UnitClass_CanFire_DeployFire_DoNotErrorFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Type->DeployFire
		&& !pThis->Type->IsSimpleDeployer
		&& !pThis->Deployed
		&& pThis->CurrentMission == Mission::Unload
	)
	{
		return 0x741327; //fireOK
	}

	return 0x0;
}

DEFINE_HOOK(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = specific_cast<UnitClass*>(pThis);

	/// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit && pUnit->Type->DeployFire
		&& !pUnit->Type->IsSimpleDeployer
		&& TechnoExt::ExtMap.Find(pThis)->DeployFireTimer.InProgress())
	{
		return DoNotExecute;
	}

	return 0x0;
}

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
DEFINE_HOOK(0x4C7518, EventClass_Execute_StopUnitDeployFire, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = specific_cast<UnitClass*>(pThis);
	if (pUnit
		&& pUnit->CurrentMission == Mission::Unload
		&& pUnit->Type->DeployFire
		&& !pUnit->Type->IsSimpleDeployer)
	{
		pUnit->SetTarget(nullptr);
		pUnit->QueueMission(Mission::Guard, true);
	}

	// Restore overridden instructions
	GET(Mission, eax, EAX);
	return eax == Mission::Construction ? 0x4C8109 : 0x4C7521;
}

DEFINE_HOOK(0x746CD0, UnitClass_SelectWeapon_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	if (pThis->Deployed && pThis->Type->DeployFire) {
		if (pThis->Type->DeployFireWeapon != -1) {
			R->EAX(pThis->Type->DeployFireWeapon);
			return 0x746CFD;
		}
	}

	R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	return 0x746CFD;
}
#endif