#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>

#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->MaxNumberOccupants > 10) {
		GET(int, nFiringIndex, EBX);
		R->EAX(BuildingTypeExt::GetOccupyMuzzleFlash(pThis,nFiringIndex));
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0x6) // A
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->MaxNumberOccupants > 10) {
		R->EDX(BuildingTypeExt::GetOccupyMuzzleFlash(pThis, pThis->FiringOccupantIndex));
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->MaxNumberOccupants > 10) {
		GET(int, nFiringIndex, EDI);
		R->ECX(BuildingTypeExt::GetOccupyMuzzleFlash(pThis, nFiringIndex));
	}

	return 0;
}

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	BuildingTypeExt::DisplayPlacementPreview();
	return 0x0;
}

DEFINE_HOOK(0x47EFAE, CellClass_Draw_It_MakePlacementGridTranparent, 0x6)
{
	LEA_STACK(BlitterFlags*, blitFlags, STACK_OFFS(0x68, 0x58));
	*blitFlags |= EnumFunctions::GetTranslucentLevel(RulesExt::Global()->PlacementGrid_TranslucentLevel);
	return 0;
}