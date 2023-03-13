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

	auto const nFireErr = pThis->GetFireError(pThis->Target, nWeapIdx, true);
	if (nFireErr == FireError::OK || nFireErr == FireError::FACING) // Deploy dire weapon does not need Facing check , duh
	{
		pThis->Fire(pThis->Target, nWeapIdx);

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

DEFINE_HOOK(0x7413B4, UnitClass_Fire_At_FireOnceNoCheck, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET_STACK(WeaponTypeClass*, pWeapon, 0x20);

	 //ignore Can_fire/FireError check for Deploy Fire Weapon !
	if (pWeapon->FireOnce && pThis->Type->DeployFire && !pThis->Type->IsSimpleDeployer && !pThis->Deployed) {
		return 0x7413CA;
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

//DEFINE_HOOK(0x41BEA0, ObjectClass_MapCoords_OffsetByFacings, 0x7)
//{
//	GET(ObjectClass*, pThis, ECX);
//	GET_STACK(CellStruct*, pResult, 0x4);
//
//	CellStruct nResult = CellClass::Coord2Cell(pThis->Location);
//	if (pThis->WhatAmI() == AbstractType::Unit)
//	{
//		auto const pUnit = static_cast<UnitClass*>(pThis);
//
//		if (pUnit->HasTurret() && pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer)
//		{
//			nResult += CellSpread::CellOfssets[pUnit->PrimaryFacing.Current().GetValue()];
//		}
//	}
//
//	*pResult = nResult;
//	R->EAX(pResult);
//	return 0x41BEDA;
//}

//DEFINE_HOOK(0x741229, UnitClass_CanFire_DeployFire, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	if (pThis->Type->DeployToFire && !pThis->Type->IsSimpleDeployer) {
//		return 0x741314;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x736DF0, UnitClass_FiringAI_DeployFire, 0x5)
//{
//	GET(UnitClass*, pThis, ECX);
//
//	if (pThis->Type->DeployToFire && !pThis->Type->IsSimpleDeployer) {
//		return 0x737146;
//	}
//
//	return 0x0;
//}
// 
//DEFINE_HOOK(0x6FF923, TechnoClass_Fire_FireOnce, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	pThis->SetTarget(nullptr);
//	pThis->QueueMission(Mission::Guard, true);
//
//	if (auto const pUnit = specific_cast<UnitClass*>(pThis)) {
//		if (pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer && pUnit->Deployed)
//			pUnit->Deployed = false;
//	}
//
//	return 0x6FF92F;
//}

//DEFINE_HOOK(0x739E4F, UnitClass_Undeploy_FireOnce, 0x7)
//{
//	return 0x0;
//}

//DEFINE_HOOK(0x74132B, UnitClass_FireError_Result, 0x7)
//{
//	GET(FireError, nRes, EAX);
//	Debug::Log("UnitClass Fire Error Result [%d] ! \n", nRes);
//	return 0x0;
//}