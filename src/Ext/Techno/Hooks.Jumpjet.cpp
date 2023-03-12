#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

#include <JumpjetLocomotionClass.h>

static JumpjetLocomotionClass* GetLoco(ILocomotion* ILoco)
{
	auto const pLoco = static_cast<LocomotionClass*>(ILoco);

	if ((((DWORD*)pLoco)[0] == JumpjetLocomotionClass::vtable))
		return static_cast<JumpjetLocomotionClass*>(pLoco);

	return nullptr;
}

static JumpjetLocomotionClass* GetLoco(LocomotionClass* pLoco)
{
	if ((((DWORD*)pLoco)[0] == JumpjetLocomotionClass::vtable))
		return static_cast<JumpjetLocomotionClass*>(pLoco);

	return nullptr;
}

// Fix [JumpjetControls] obsolete in RA2/YR
// Author: Uranusian
DEFINE_HOOK(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	const auto pRules = RulesClass::Instance();
	const auto pRulesExt = RulesExt::Global();

	pThis->JumpjetTurnRate = pRules->TurnRate;
	pThis->JumpjetSpeed = pRules->Speed;
	pThis->JumpjetClimb = static_cast<float>(pRules->Climb);
	pThis->JumpjetCrash = static_cast<float>(pRulesExt->JumpjetCrash.Get());
	pThis->JumpjetHeight = pRules->CruiseHeight;
	pThis->JumpjetAccel = static_cast<float>(pRules->Acceleration);
	pThis->JumpjetWobbles = static_cast<float>(pRules->WobblesPerSecond);
	pThis->JumpjetNoWobbles = pRulesExt->JumpjetNoWobbles.Get();
	pThis->JumpjetDeviation = pRules->WobbleDeviation;

	return 0x711601;
}

// skip vanilla JumpjetControls and make it earlier load
DEFINE_JUMP(LJMP, 0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls

DEFINE_HOOK(0x52D0F9, InitRules_EarlyLoadJumpjetControls, 0x6)
{
	GET(RulesClass*, pThis, ECX);
	GET(CCINIClass*, pINI, EAX);

	RulesExt::LoadEarlyBeforeColor(pThis, pINI);
	pThis->Read_JumpjetControls(pINI);

	return 0;
}

DEFINE_HOOK(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	if (const auto pRulesExt = RulesExt::Global())
	{
		GET(CCINIClass*, pINI, EDI);

		INI_EX exINI(pINI);

		pRulesExt->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls(), "Crash");
		pRulesExt->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls(), "NoWobbles");
	}

	return 0;
}

DEFINE_HOOK(0x54C036, JumpjetLocomotionClass_State3_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	if (pLinkedTo->GetTechnoType()->SensorsSight && pLinkedTo->LastJumpjetMapCoords != currentCell)
	{
		pLinkedTo->RemoveSensorsAt(pLinkedTo->LastJumpjetMapCoords);
		pLinkedTo->AddSensorsAt(currentCell);
	}

	return 0;
}

//DEFINE_HOOK(0x54B8E9, JumpjetLocomotionClass_In_Which_Layer_Deviation, 0x6)
//{
//	GET(TechnoClass*, pThis, EAX);
//
//	if (pThis->IsInAir())
//	{
//		if (!TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->JumpjetAllowLayerDeviation
//			.Get(RulesExt::Global()->JumpjetAllowLayerDeviation.Get()))
//		{
//			R->EDX(INT32_MAX); // Override JumpjetHeight / CruiseHeight check so it always results in 3 / Layer::Air.
//			return 0x54B96B;
//		}
//	}
//
//	return 0;
//}

DEFINE_HOOK(0x54D138, JumpjetLocomotionClass_Movement_AI_SpeedModifiers, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (auto const pLinked = pThis->LinkedTo)
	{
		const double multiplier = TechnoExt::GetCurrentSpeedMultiplier(pLinked);
		pThis->Speed = Game::F2I(pLinked->GetTechnoType()->JumpjetSpeed * multiplier);
	}

	return 0;
}

DEFINE_HOOK(0x54CB0E, JumpjetLocomotionClass_State5_CrashRotation, 0x7)
{
	GET(JumpjetLocomotionClass*, pLoco, EDI);

	bool bRotate = RulesExt::Global()->JumpjetCrash_Rotate.Get();

	if (auto const pOwner = pLoco->LinkedTo ? pLoco->LinkedTo : pLoco->Owner)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		bRotate = pTypeExt->JumpjetCrash_Rotate.Get(bRotate);
	}

	return bRotate ? 0 : 0x54CB3E;

}

// Bugfix: Jumpjet turn to target when attacking

// Jumpjets stuck at FireError::FACING because WW didn't use a correct facing
DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_FireErrorIsFACING, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
	{
		R->EAX(pType);
		return 0x736F7E;
	}
	
	CoordStruct& source = pThis->Location;
	CoordStruct target = pThis->Target->GetCoords(); // Target checked so it's not null here
	DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };

	if (pType->Turret && !pType->HasTurret) // 0x736F92
	{
		pThis->SecondaryFacing.Set_Desired(tgtDir);
	}
	else // 0x736FB6
	{
		if (auto jjLoco = GetLoco(pThis->Locomotor.get()))
		{
			//wrong destination check and wrong Is_Moving usage for jumpjets, should have used Is_Moving_Now
			if (jjLoco->NextState != JumpjetLocomotionClass::State::Cruising)
			{
				jjLoco->Facing.Set_Desired(tgtDir);
				pThis->PrimaryFacing.Set_Desired(tgtDir);
				pThis->SecondaryFacing.Set_Desired(tgtDir);
			}
		}
		else if (!pThis->Destination && !pThis->Locomotor->Is_Moving())
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

	if ((pType->DeployFire || TechnoExt::GetDeployFireWeapon(pThis) == wpIdx) && pThis->CurrentMission == Mission::Unload)
		return 0;

	auto const pWpnStruct = pThis->GetWeapon(wpIdx);
	if(!pWpnStruct)
		return 0;

	auto const pWpn = pWpnStruct->WeaponType;
	if (pWpn->OmniFire)
	{
		const auto pTypeExt = WeaponTypeExt::ExtMap.Find(pWpn);
		if (pTypeExt->OmniFire_TurnToTarget.Get() && !pThis->Locomotor->Is_Moving_Now())
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pThis->Target->GetCoords();
			DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };

			if (pThis->GetRealFacing().Current() != tgtDir)
			{
				if (auto const pLoco = GetLoco(pThis->Locomotor.get()))
					pLoco->Facing.Set_Desired(tgtDir);
				else
					pThis->PrimaryFacing.Set_Desired(tgtDir);
			}
		}
	}

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_TemporaryFix, 0x6)
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

// These are subject to changes if someone wants to properly implement jumpjet tilting
DEFINE_HOOK(0x54DCCF, JumpjetLocomotionClass_DrawMatrix_TiltCrashJumpjet, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	//if (static_cast<JumpjetLocomotionClass*>(iloco)->State < JumpjetLocomotionClass::State::Crashing)
	if (static_cast<JumpjetLocomotionClass*>(iloco)->NextState == JumpjetLocomotionClass::State::Grounded)
		return 0x54DCE8;

	return 0;
}

DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	// I still don't know why jumpjet loco behaves differently for the moment
	// so I don't check jumpjet loco or InAir here, feel free to change if it doesn't break performance.
	if (!pThis->Target && !pThis->Type->TurretSpins && (pThis->Type->JumpJet || pThis->Type->BalloonHover))
	{
		pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
		pThis->TurretIsRotating = pThis->SecondaryFacing.Is_Rotating();
		return 0x736C09;
	}

	return 0;
}

/*
DEFINE_HOOK(0x54DD3D, JumpjetLocomotionClass_DrawMatrix_AxisCenterInAir, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	auto state = static_cast<JumpjetLocomotionClass*>(iloco)->State;
	if (state && state < JumpjetLocomotionClass::State::Crashing)
		return  0x54DE88;
	return 0;
}
*/

//TODO : Issue #690 #655

// Otamaa
//DEFINE_HOOK(0x54DCE8, JumpetLocomotionClass_DrawMatrix, 0x9)
//{
//	GET(ILocomotion*, pILoco, ESI);
//	auto pLoco = static_cast<JumpjetLocomotionClass*>(pILoco);
//
//	if (YRComPtr<IPiggyback> pPiggy = pLoco->Owner->Locomotor)
//	{
//
//	}
//
//	return LocomotionClass::End_Piggyback(pLoco->Owner->Locomotor) ? 0x0 : 0x54DF13;
//}

DEFINE_HOOK(0x518313, InfantryClass_ReceiveDamage_JumpjetExplode, 0x6)
{
	enum { PlayDeadSequence = 0x518362 , ContinueCheckForJumpJet = 0x5185F1  };
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pThisType, EAX);

	if (pThisType->JumpJet ) {

		if(pThisType->Explodes)
			TechnoExt::PlayAnim(RulesClass::Instance->InfantryExplode, pThis);

		return ContinueCheckForJumpJet;
	}

	return PlayDeadSequence;
}

DEFINE_HOOK(0x54D208, JumpjetLocomotionClass_MovementAI_Wobbles, 0x5)
{
	enum
	{
		SetWobble = 0x54D20F,
		NoWobble = 0x54D22C
	};

	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (pThis->NoWobbles)
		return NoWobble;

	if (const auto pUnit = specific_cast<UnitClass*>(pThis->LinkedTo))
		return pUnit->IsDeactivated() ? NoWobble : SetWobble;

	return SetWobble;
}


// Bugfix: Jumpjet turn to target when attacking
// Even though it's still not the best place to do this, given that 0x54BF5B has done the similar action, I'll do it here too
//DEFINE_HOOK(0x54BD93, JumpjetLocomotionClass_State2_54BD30_TurnToTarget, 0x6)
//{
//	enum { ContinueNoTarget = 0x54BDA1, EndFunction = 0x54BFDE, ContinueFunc = 0x54BDA1 };
//
//	GET(JumpjetLocomotionClass* const, pLoco, ESI);
//	GET(FootClass* const, pLinkedTo, EDI);
//
//	const auto pTarget = pLinkedTo->Target;
//	if (!pTarget)
//		return ContinueNoTarget;
//
//	if (const auto pThis = abstract_cast<UnitClass*>(pLinkedTo))
//	{
//		if (TechnoTypeExt::ExtMap.Find(pThis->Type)->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
//		{
//			CoordStruct& source = pThis->Location;
//			CoordStruct target = pTarget->GetCoords();
//			DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));
//
//			if (pThis->GetRealFacing().Current().GetFacing<32>() != tgtDir.GetFacing<32>())
//				pLoco->Facing.Set_Desired(tgtDir);
//
//			R->EAX(pTarget);
//			return EndFunction;
//		}
//	}
//
//	return ContinueFunc;
//}

// Jumpjets stuck at FACING fireerror because WW didn't use a correct facing
//DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_Ferr_FACING_Jumpjet, 0x6)
//{
//	GET(UnitClass* const, pThis, ESI);
//
//	if (auto const pLoco = GetLocomotorType<JumpjetLocomotionClass,false>(pThis))
//	{
//		CoordStruct& source = pThis->Location;
//		CoordStruct target = pThis->Target->GetCoords();
//		DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };
//
//		if (pLoco->IsMoving && pThis->Type->Turret && !pThis->Type->HasTurret) // 0x736F92
//			pThis->SecondaryFacing.Set_Desired(tgtDir);
//		else //0x736FB6: Jumpjets often have destination even if not moving, detailed in the turret issue below
//		{
//			pThis->PrimaryFacing.Set_Desired(tgtDir);
//			pThis->SecondaryFacing.Set_Desired(tgtDir);
//			pLoco->Facing.Set_Desired(tgtDir);
//		}
//
//		return 0x736FB1;
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x736EE9, UnitClass_UpdateFiring_Ferr_OK_OmniFire_Facing, 0x6)
//{
//	GET(UnitClass* const, pThis, ESI);
//	GET(int const, wpIdx, EDI);
//
//	//if (pThis->GetWeapon(wpIdx)->WeaponType->OmniFire)
//	//{
//	//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//	//	if (pTypeExt->OmniFireTurnToTarget.Get())
//	//	{
//	//		CoordStruct& source = pThis->Location;
//	//		CoordStruct target = pThis->Target->GetCoords();
//	//		DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };
//
//	//		if (pThis->GetRealFacing().Current().GetFacing<32>() != tgtDir.GetFacing<32>())
//	//		{
//	//			if (auto const pLoco = GetLocomotor<JumpjetLocomotionClass*>(pThis))
//	//				pLoco->Facing.Set_Desired(tgtDir);
//	//			else
//	//				pThis->PrimaryFacing.Set_Desired(tgtDir);
//	//		}
//	//	}
//	//}
//
//	return 0;
//}

//DEFINE_HOOK(0x54AEC0, JumpjetLocomotionClass_Process_TurnToTarget, 0x8)
//{
	//GET_STACK(ILocomotion*, iLoco, 0x4);
//	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
//const auto pThis = pLoco->Owner;
//	const auto pType = pThis->GetTechnoType();
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pType);
//
//	if (pTypeExt && pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget) &&
	//	pThis->WhatAmI() == AbstractType::Unit && pThis->IsInAir() && !pType->TurretSpins && pLoco)
	//{
	//	if (const auto pTarget = pThis->Target)
	//	{
//			const CoordStruct source = pThis->Location;
	//		const CoordStruct target = pTarget->GetCoords();
//			const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));
//			if (pThis->GetRealFacing().current().value32() != tgtDir.value32())
	//			pLoco->Facing.turn(tgtDir);
//		}
//	}
//	return 0;
//}

// The ingame behavior looks not better than the previous fix
//DEFINE_HOOK_AGAIN(0x736FE8, UnitClass_UpdateFiring_736DF0_JumpjetFacing, 0x6)// Turret and FireError == FACING
//DEFINE_HOOK(0x736EE9, UnitClass_UpdateFiring_736DF0_JumpjetFacing, 0x6) // FireError == OK
//{
//	GET(UnitClass* const, pThis, ESI);
//	ILocomotion* iloco = pThis->Locomotor.get();
//	CLSID locoCLSID;
//	if (SUCCEEDED(static_cast<LocomotionClass*>(iloco)->GetClassID(&locoCLSID)) && locoCLSID == LocomotionClass::CLSIDs::Jumpjet)
//	{
//		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//		if (pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
//		{
//			auto const pLoco = static_cast<JumpjetLocomotionClass*>(iloco);
//			CoordStruct& source = pThis->Location;
//			CoordStruct target = pThis->Target->GetCoords();
//			DirStruct const tgtDir = DirStruct { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };
//
//			pLoco->Facing.Set_Desired(tgtDir);
//
//			if (R->Origin() == 0x736FE8)
//			{
//				pThis->SecondaryFacing.Set_Desired(tgtDir);
//				return 0x737021;
//			}
//		}
//	}
//	return 0;
//}