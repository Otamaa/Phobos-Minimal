#include <Utilities/LocomotionCast.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

// Bugfix: Jumpjet turn to target when attacking
// Even though it's still not the best place to do this, given that 0x54BF5B has done the similar action, I'll do it here too
DEFINE_HOOK(0x54BD93, JumpjetLocomotionClass_State2_54BD30_TurnToTarget, 0x6)
{
	enum { ContinueNoTarget = 0x54BDA1, EndFunction = 0x54BFDE, ContinueFunc = 0x54BDA1 };

	GET(JumpjetLocomotionClass* const, pLoco, ESI);
	GET(FootClass* const, pLinkedTo, EDI);

	const auto pTarget = pLinkedTo->Target;
	if (!pTarget)
		return ContinueNoTarget;

	if (const auto pThis = abstract_cast<UnitClass*>(pLinkedTo))
	{
		if (TechnoTypeExt::ExtMap.Find(pThis->Type)->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pTarget->GetCoords();
			DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

			if (pThis->GetRealFacing().Current().GetFacing<32>() != tgtDir.GetFacing<32>())
				pLoco->Facing.Set_Desired(tgtDir);

			R->EAX(pTarget);
			return EndFunction;
		}
	}

	return ContinueFunc;
}

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