#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>


//bool __fastcall CanUpdate(UnitClass* pThis, void*) {
//	return pThis->DeathFrameCounter != -1 || pThis->IsDeactivated();
//}

//DEFINE_JUMP(CALL,0x7360C9, GET_OFFSET(CanUpdate));


DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	if (!pBuilding || !pBuilding->Owner)
		return 0;

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

DEFINE_HOOK(0x073B061, UnitClass_PerCellProcess_TiltWhenCrushes, 0x6)
{
	enum { SkipTilt = 0x73B067 };

	GET(UnitClass*, pThis, EBP);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Walls.Get(pThis->Type->TiltsWhenCrushes))
		return SkipTilt;

	return 0;
}

DEFINE_HOOK(0x0741941, UnitClass_OverrunSquare_TiltWhenCrushes, 0x6)
{
	enum { SkipTilt = 0x74195E };

	GET(UnitClass*, pThis, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->TiltsWhenCrushes_Vehicles.Get(pThis->Type->TiltsWhenCrushes))
		return SkipTilt;

	return 0;
}

DEFINE_HOOK(0x4B1150, DriveLocomotionClass_WhileMoving_WallCrushSlowdown, 0x9)
{
	enum { SkipSlowdown = 0x4B1182 };

	GET(FootClass*, pLinkedTo, ECX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (!pTypeExt->WallCrushSlowdown)
		return SkipSlowdown;

	return 0;
}

DEFINE_HOOK(0x6A0813, ShipLocomotionClass_WhileMoving_WallCrushSlowdown, 0x9)
{
	enum { SkipSlowdown = 0x6A0845 };

	GET(FootClass*, pLinkedTo, ECX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (!pTypeExt->WallCrushSlowdown)
		return SkipSlowdown;

	return 0;
}