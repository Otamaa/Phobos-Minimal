#include <Utilities/LocomotionCast.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>


// Jumpjets stuck at FACING fireerror because WW didn't use a correct facing
DEFINE_HOOK(0x736F78, UnitClass_UpdateFiring_Ferr_FACING_Jumpjet, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	if (auto const pLoco = GetLocomotorType<JumpjetLocomotionClass,false>(pThis))
	{
		CoordStruct& source = pThis->Location;
		CoordStruct target = pThis->Target->GetCoords();
		DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };

		if (pLoco->IsMoving && pThis->Type->Turret && !pThis->Type->HasTurret) // 0x736F92
			pThis->SecondaryFacing.Set_Desired(tgtDir);
		else //0x736FB6: Jumpjets often have destination even if not moving, detailed in the turret issue below
		{
			pThis->PrimaryFacing.Set_Desired(tgtDir);
			pThis->SecondaryFacing.Set_Desired(tgtDir);
			pLoco->Facing.Set_Desired(tgtDir);
		}

		return 0x736FB1;
	}

	return 0;
}

DEFINE_HOOK(0x736EE9, UnitClass_UpdateFiring_Ferr_OK_OmniFire_Facing, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int const, wpIdx, EDI);

	//if (pThis->GetWeapon(wpIdx)->WeaponType->OmniFire)
	//{
	//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	//	if (pTypeExt->OmniFireTurnToTarget.Get())
	//	{
	//		CoordStruct& source = pThis->Location;
	//		CoordStruct target = pThis->Target->GetCoords();
	//		DirStruct tgtDir { Math::atan2(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X)) };

	//		if (pThis->GetRealFacing().Current().GetFacing<32>() != tgtDir.GetFacing<32>())
	//		{
	//			if (auto const pLoco = GetLocomotor<JumpjetLocomotionClass*>(pThis))
	//				pLoco->Facing.Set_Desired(tgtDir);
	//			else
	//				pThis->PrimaryFacing.Set_Desired(tgtDir);
	//		}
	//	}
	//}

	return 0;
}
