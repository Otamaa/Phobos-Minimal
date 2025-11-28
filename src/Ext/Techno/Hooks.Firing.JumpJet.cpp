#include "Body.h"

#include <Ext/WeaponType/Body.h>

#include <Locomotor/LocomotionClass.h>
#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

#pragma region JJFixes
// Bugfix: Jumpjet turn to target when attacking
// Jumpjets stuck at FireError::FACING because WW didn't use a correct facing
ASMJIT_PATCH(0x736F78, UnitClass_UpdateFiring_FireErrorIsFACING, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pType = pThis->Type;
	CoordStruct& source = pThis->Location;
	CoordStruct target = pThis->Target->GetCoords(); // Target checked so it's not null here
	const DirStruct tgtDir { (double)(source.Y - target.Y), (double)(target.X - source.X) };

	if (pType->Turret && !pType->HasTurret) // 0x736F92
	{
		pThis->SecondaryFacing.Set_Desired(tgtDir);
	}
	else // 0x736FB6
	{
		if (auto jjLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pThis->Locomotor))
		{
			//wrong destination check and wrong Is_Moving usage for jumpjets, should have used Is_Moving_Now
			if (jjLoco->NextState != JumpjetLocomotionClass::State::Cruising)
			{
				jjLoco->Facing.Set_Desired(tgtDir);
				pThis->PrimaryFacing.Set_Desired(tgtDir);
				pThis->SecondaryFacing.Set_Desired(tgtDir);
			}
		}
		else if (!pThis->Destination && !pThis->Locomotor.GetInterfacePtr()->Is_Moving())
		{
			pThis->PrimaryFacing.Set_Desired(tgtDir);
			pThis->SecondaryFacing.Set_Desired(tgtDir);
		}
	}

	return 0x736FB1;
}

ASMJIT_PATCH(0x736E6E, UnitClass_UpdateFiring_OmniFireTurnToTarget, 0x9) {

	GET(FireError, err, EBP);
	GET(UnitClass* const, pThis, ESI);
	GET(int const, wpIdx, EDI);

	if (pThis->IsWarpingIn() || err != FireError::OK && err != FireError::REARM)
		return 0;

	auto pType = pThis->Type;

	if ((pType->Turret && !pType->HasTurret) || pType->TurretSpins)
		return 0;

	if ((pType->DeployFire || pType->DeployFireWeapon == wpIdx) && pThis->CurrentMission == Mission::Unload)
		return 0;

	auto const pWpn = pThis->GetWeapon(wpIdx)->WeaponType;
	if (pWpn->OmniFire)
	{
		const auto pTypeExt = WeaponTypeExtContainer::Instance.Find(pWpn);
		if (pTypeExt->OmniFire_TurnToTarget.Get() && !pThis->Locomotor->Is_Moving_Now())
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pThis->Target->GetCoords();
			DirStruct tgtDir { std::atan2(double(source.Y - target.Y), double(target.X - source.X)) };

			if (pThis->GetRealFacing() != tgtDir)
			{
				if (auto const pLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
					pLoco->Facing.Set_Desired(tgtDir);
				else
					pThis->PrimaryFacing.Set_Desired(tgtDir);
			}
		}
	}

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
// ASMJIT_PATCH_AGAIN(0x54BB80, JumpjetLocomotionClass_ProcessState_ClearDestination, 0x7) // Process_Ascending
// ASMJIT_PATCH_AGAIN(0x54C2CB, JumpjetLocomotionClass_ProcessState_ClearDestination, 0x7) // Process_Cruising
// ASMJIT_PATCH_AGAIN(0x54C376, JumpjetLocomotionClass_ProcessState_ClearDestination, 0x7) // Process_Cruising
// ASMJIT_PATCH(0x54C64B, JumpjetLocomotionClass_ProcessState_ClearDestination, 0x7) // Process_Descending
// {
// 	GET(FootClass* const, pThis, ECX);
//
// 	if (pThis->Destination == pThis->GetCell()) {
// 		pThis->SetDestination(nullptr, true);
// 	}
//
// 	return 0;
// }

//
//ASMJIT_PATCH(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	// I still don't know why jumpjet loco behaves differently for the moment
//	// so I don't check jumpjet loco or InAir here, feel free to change if it doesn't break performance.
//	if (!pThis->Target && !pThis->Type->TurretSpins && (pThis->Type->JumpJet || pThis->Type->BalloonHover))
//	{
//		pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
//		pThis->TurretIsRotating = pThis->SecondaryFacing.Is_Rotating();
//		return 0x736C09;
//	}
//
//	return 0;
//}

// Man, what can I say
//ASMJIT_PATCH(0x54D67B, JumpjetLocomotionClass_ProcessMove_NotJumpjetTurn, 0x5)
//{
//	GET(JumpjetLocomotionClass*, pThis, ESI);
//
//	pThis->LinkedTo->PrimaryFacing.Set_Desired(pThis->Facing.Desired());
//
//	return 0x54D697;
//}

#pragma endregion