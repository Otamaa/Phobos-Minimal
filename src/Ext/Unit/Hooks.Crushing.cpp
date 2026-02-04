#include <Locomotor/DriveLocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>
#include <New/Interfaces/AdvancedDriveLocomotionClass.h>

ASMJIT_PATCH(0x073B05B, UnitClass_PerCellProcess_TiltWhenCrushes, 0x6)
{
	enum { SkipGameCode = 0x73B074 };

	GET(UnitClass*, pThis, EBP);
	GET(CellClass*, pCell, EDI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Overlays.Get(pThis->Type->TiltsWhenCrushes))
		return SkipGameCode;

	if (pTypeExt->CrushOverlayExtraForwardTilt.isset())
		pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushOverlayExtraForwardTilt.Get(-0.05));

	if (AdvancedDriveLocomotionClass::IsReversing(pThis))
		pThis->RockingForwardsPerFrame -= static_cast<float>(pTypeExt->CrushOverlayExtraForwardTilt.Get(-0.05));
	else
		pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushOverlayExtraForwardTilt.Get(-0.05));

	if (pCell->OverlayTypeIndex == -1)
		TechnoClass::ClearWhoTargetingThis(pCell);

	return SkipGameCode;
}

ASMJIT_PATCH(0x0741941, UnitClass_OverrunSquare_TiltWhenCrushes, 0x6)
{
	enum { SkipGameCode = 0x74195E };

	GET(UnitClass*, pThis, EDI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Vehicles.Get(pThis->Type->TiltsWhenCrushes))
		return SkipGameCode;

	if (pTypeExt->CrushForwardTiltPerFrame.isset())
	{
		if (AdvancedDriveLocomotionClass::IsReversing(pThis))
			pThis->RockingForwardsPerFrame -= static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get());
		else
			pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get());
	}
	else
	{
		if (AdvancedDriveLocomotionClass::IsReversing(pThis))
			pThis->RockingForwardsPerFrame -= static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.05));
		else
			pThis->RockingForwardsPerFrame += static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.05));
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x4B1150, DriveLocomotionClass_WhileMoving_CrushSlowdown, 0x9)
{
	enum { SkipGameCode = 0x4B116B };

	GET(DriveLocomotionClass*, pThis, EBP);

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis->LinkedTo);
	auto slowdownCoefficient = pThis->movementspeed_50;

	if (slowdownCoefficient > pTypeExt->CrushSlowdownMultiplier)
		slowdownCoefficient = pTypeExt->CrushSlowdownMultiplier;

	__asm { fld slowdownCoefficient };

	return SkipGameCode;

}

ASMJIT_PATCH(0x4B19F7, DriveLocomotionClass_WhileMoving_CrushTilt, 0xD)
{
	enum { SkipGameCode1 = 0x4B1A04, SkipGameCode2 = 0x4B1A58 };

	GET(DriveLocomotionClass*, pThis, EBP);

	auto const pLinkedTo = pThis->LinkedTo;
	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);

	pLinkedTo->RockingForwardsPerFrame = static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get(-0.05));
	return R->Origin() == 0x4B19F7 ? SkipGameCode1 : SkipGameCode2;
}
ASMJIT_PATCH_AGAIN(0x4B1A4B, DriveLocomotionClass_WhileMoving_CrushTilt, 0xD)

ASMJIT_PATCH(0x6A0813, ShipLocomotionClass_WhileMoving_CrushSlowdown, 0x9)
{
	enum { SkipGameCode = 0x6A082E };

	GET(ShipLocomotionClass*, pThis, EBP);

	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis->LinkedTo);
	auto slowdownCoefficient = pThis->movementspeed_50;

	if (slowdownCoefficient > pTypeExt->CrushSlowdownMultiplier)
		slowdownCoefficient = pTypeExt->CrushSlowdownMultiplier;

	__asm { fld slowdownCoefficient };

	return SkipGameCode;
}

ASMJIT_PATCH(0x6A108D, ShipLocomotionClass_WhileMoving_CrushTilt, 0xD)
{
	enum { SkipGameCode = 0x6A109A };

	GET(ShipLocomotionClass*, pThis, EBP);

	auto const pLinkedTo = pThis->LinkedTo;
	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);

	if (pTypeExt->CrushForwardTiltPerFrame.isset()) {
		pLinkedTo->RockingForwardsPerFrame = static_cast<float>(pTypeExt->CrushForwardTiltPerFrame.Get());
		return SkipGameCode;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4B1146, SomeLocomotionClass_WhileMoving_SkipCrushSlowDown, 0x6) // Drive
{
	GET(FootClass*, pLinkedTo, ECX);
	auto const pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);
	return pTypeExt->SkipCrushSlowdown ? R->Origin() + 0x3C : 0;
}
ASMJIT_PATCH_AGAIN(0x6A0809, SomeLocomotionClass_WhileMoving_SkipCrushSlowDown, 0x6) // Ship