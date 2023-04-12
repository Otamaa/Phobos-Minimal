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
	return pThis->Type->DeployFireWeapon == -1 ? 0x52194E : 0x0;
}

DEFINE_HOOK(0x73DD12, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C };
	GET(UnitClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto const nWeapIdx = TechnoExt::GetDeployFireWeapon(pThis);
	auto const pWs = pThis->GetWeapon(nWeapIdx);
	auto const pWp = pWs->WeaponType;

	if (!pWp || (pWp->FireOnce && pExt->DeployFireTimer.GetTimeLeft() > 0))
	{
		pExt->Get()->SetTarget(nullptr);
		pExt->Get()->QueueMission(Mission::Guard, true);
		return SkipGameCode;
	}

	if (pThis->GetFireError(pThis->Target, nWeapIdx, true) == FireError::OK)
	{
		pThis->Fire(pThis->GetCell(), nWeapIdx);

		// fire error facing will stop techno firing , so force it to target cell instead
		if (pWs->WeaponType->FireOnce)
		{
			pThis->SetTarget(nullptr);
			pThis->QueueMission(Mission::Guard, true);
			pExt->DeployFireTimer.Start(pWs->WeaponType->ROF);
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x741288, UnitClass_CanFire_DeployFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Type->DeployFire 
		&& !pThis->Type->IsSimpleDeployer 
		&& !pThis->Deployed) {
		if (!pTypeExt->DeployFire_UpdateFacing) {
			//R->EAX(FireError::OK); //yes , dont return facing error 
			//return 0x74132B;
			return 0x741327; //fireOK
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = specific_cast<UnitClass*>(pThis);

	if (!pUnit)
		return 0x0;

	/// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer)
	{
		auto const nWeapIdx = TechnoExt::GetDeployFireWeapon(pUnit);
		auto const pWs = pThis->GetWeapon(nWeapIdx);

		if (pWs->WeaponType && pWs->WeaponType->FireOnce
			&& TechnoExt::ExtMap.Find(pThis)->DeployFireTimer.HasTimeLeft())
			return DoNotExecute;
	}

	return 0x0;
}

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
DEFINE_HOOK(0x4C7518, EventClass_Execute_StopUnitDeployFire, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);
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

	const auto pType = pThis->Type;

	if (pThis->Deployed && pType->DeployFire)
	{
		int weaponIndex = pType->DeployFireWeapon;

		if (weaponIndex != -1)
		{
			R->EAX(weaponIndex);
			return 0x746CFD;
		}
	}

	R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	return 0x746CFD;
}