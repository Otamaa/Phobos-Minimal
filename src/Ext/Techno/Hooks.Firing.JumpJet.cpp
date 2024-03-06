#include "Body.h"

#include <Ext/WeaponType/Body.h>

#include <Locomotor/LocomotionClass.h>
#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

#pragma region JJFixes
// Bugfix: Jumpjet turn to target when attacking
// Jumpjets stuck at FireError::FACING because WW didn't use a correct facing
DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_FireErrorIsFACING, 0x6)
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

// For compatibility with previous builds
DEFINE_HOOK(0x736EE9, UnitClass_UpdateFiring_FireErrorIsOK, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int const, wpIdx, EDI);
	auto pType = pThis->Type;

	if ((pType->Turret && !pType->HasTurret) || pType->TurretSpins)
		return 0;

	if ((pType->DeployFire || TechnoExtData::GetDeployFireWeapon(pThis) == wpIdx) && pThis->CurrentMission == Mission::Unload)
		return 0;

	auto const pWpnStruct = pThis->GetWeapon(wpIdx);
	if (!pWpnStruct)
		return 0;

	auto const pWpn = pWpnStruct->WeaponType;
	if (pWpn->OmniFire)
	{
		const auto pTypeExt = WeaponTypeExtContainer::Instance.Find(pWpn);
		if (pTypeExt->OmniFire_TurnToTarget.Get() && !pThis->Locomotor.GetInterfacePtr()->Is_Moving_Now())
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pThis->Target->GetCoords();
			const DirStruct tgtDir { double(source.Y - target.Y), double(target.X - source.X) };

			if (pThis->GetRealFacing() != tgtDir)
			{
				if (auto jjLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pThis->Locomotor))
				{
					jjLoco->Facing.Set_Desired(tgtDir);
				}
				else
					pThis->PrimaryFacing.Set_Desired(tgtDir);
			}
		}
	}

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_Jumpjet, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipCheckDestination = 0x736BCA, GetDirectionTowardsDestination = 0x736BBB };
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	const auto pType = pThis->Type;
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily, and in most cases Jumpjet/BallonHover should cover most of it
	if (!pType->TurretSpins && (pType->JumpJet || pType->BalloonHover))
		return SkipCheckDestination;

	return 0;
}

//
//DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
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
DEFINE_HOOK(0x54D67B, JumpjetLocomotionClass_ProcessMove_NotJumpjetTurn, 0x5)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	pThis->LinkedTo->PrimaryFacing.Set_Desired(pThis->Facing.Desired());

	return 0x54D697;
}

#pragma endregion