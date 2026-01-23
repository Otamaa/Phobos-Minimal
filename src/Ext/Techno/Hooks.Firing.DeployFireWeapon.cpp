#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>

//Author : Otamaa
ASMJIT_PATCH(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	int weapon = pThis->Type->DeployFireWeapon;
	if (pThis->Type->DeployFireWeapon == -1) {
		if (pThis->Target && (pThis->Target->WhatAmI() == CellClass::AbsID || pThis->Target->AbstractFlags & AbstractFlags::Techno && ((TechnoClass*)pThis->Target)->IsAlive)) {
			weapon = pThis->SelectWeapon(pThis->Target);
		} else {
			weapon = 0;
		}
	}

	R->EDI(weapon);
	return 0x5223B9;
}

#include <Ext/InfantryType/Body.h>

ASMJIT_PATCH(0x5218F3, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->DeployFireWeapon == -1)
		return 0x52194E;

	if (pThis->Type->IsGattling || TechnoTypeExtContainer::Instance.Find(pThis->Type)->MultiWeapon.Get())
		return !pThis->IsDeployed() ? 0x52194E : 0x52190D;

	if(pThis->IsDeployed())
		return 0x52190D;

	return 0x521917;
}

//fuckin broken !
//DEFINE_FUNCTION_JUMP(LJMP , 0x5218E0 , FakeInfantryClass::_SelectWeaponAgainst)
//DEFINE_FUNCTION_JUMP(VTABLE , 0x7EB33C , FakeInfantryClass::_SelectWeaponAgainst)

#ifndef DISABLEFORTESTINGS
ASMJIT_PATCH(0x6FF923, TechnoClass_FireaAt_FireOnce, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	pThis->SetTarget(nullptr);
	if (auto pUnit = cast_to<UnitClass*, false>(pThis)) {
		if (pUnit->Type->DeployFire
			&& !pUnit->Type->IsSimpleDeployer
			&& !pUnit->Deployed
			&& pThis->CurrentMission == Mission::Unload
		)
		{
			TechnoExtContainer::Instance.Find(pUnit)->DeployFireTimer.Start(pWeapon->ROF);
		}
	}

	return 0x6FF92F;
}

ASMJIT_PATCH(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C, SetMissionGuard = 0x73DEBA };

	GET(UnitClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pExt->DeployFireTimer.InProgress())
	{
		auto const nWeapIdx = TechnoExtData::GetDeployFireWeapon(pThis);
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

ASMJIT_PATCH(0x741288, UnitClass_CanFire_DeployFire_DoNotErrorFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

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

ASMJIT_PATCH(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = cast_to<UnitClass*, false>(pThis);

	/// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit && pUnit->Type->DeployFire
		&& !pUnit->Type->IsSimpleDeployer
		&& TechnoExtContainer::Instance.Find(pThis)->DeployFireTimer.InProgress())
	{
		return DoNotExecute;
	}

	return 0x0;
}

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
ASMJIT_PATCH(0x4C7518, EventClass_Execute_StopUnitDeployFire, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = cast_to<UnitClass*, false>(pThis);
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

ASMJIT_PATCH(0x746CD0, UnitClass_SelectWeapon_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	//GET_STACK(uintptr_t, callerAddress, 0x0);

	if (pThis->Deployed && pThis->Type->DeployFire) {
		if (pThis->Type->DeployFireWeapon != -1) {
			R->EAX(pThis->Type->DeployFireWeapon);
			return 0x746CFD;
		}
	}

	//if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
	//	if(!pObj->IsAlive) {
	//		//Debug::LogInfo("[{}] {} {} Attempt to target death Object of {}!"
	//		//	, callerAddress ,(void*)pThis , pThis->get_ID() , (void*)pTarget);

	//		pTarget = nullptr;
	//	}
	//}

	R->EAX(pThis->TechnoClass::SelectWeapon(pTarget));
	return 0x746CFD;
}
#endif

ASMJIT_PATCH(0x51ECC0, InfantryClass_MouseOverObject_IsAreaFire, 0xA)
{
	enum { IsAreaFire = 0x51ECE5, NotAreaFire = 0x51ECEC };

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	const int deployWeaponIdx = pThis->Type->DeployFireWeapon;
	const auto deployWeapon = pThis->GetWeapon(deployWeaponIdx >= 0 ? deployWeaponIdx : pThis->SelectWeapon(pObject))->WeaponType;

	return deployWeapon && deployWeapon->AreaFire ? IsAreaFire : NotAreaFire;
}

ASMJIT_PATCH(0x6F7666, TechnoClass_TriggersCellInset_DeployWeapon, 0x8)
{
	enum { NotAreaFire = 0x6F7776, ContinueIn = 0x6F7682 };

	GET(TechnoClass*, pThis, ESI);
	int weaponIdx;

	if (const auto pInfantry = cast_to<InfantryClass*>(pThis))
	{
		GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x28, 0x4));
		const int deployWeaponIdx = pInfantry->Type->DeployFireWeapon;
		weaponIdx = deployWeaponIdx >= 0 ? deployWeaponIdx : pThis->SelectWeapon(pTarget);
	}
	else
	{
		weaponIdx = pThis->IsNotSprayAttack();
	}

	const auto deployWeaponStruct = pThis->GetWeapon(weaponIdx);
	return deployWeaponStruct && deployWeaponStruct->WeaponType && deployWeaponStruct->WeaponType->AreaFire ? ContinueIn : NotAreaFire;
}