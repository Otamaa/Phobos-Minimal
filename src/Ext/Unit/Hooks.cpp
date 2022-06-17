#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>


//bool __fastcall CanUpdate(UnitClass* pThis, void*) {
//	return pThis->DeathFrameCounter != -1 || pThis->IsDeactivated();
//}

//DEFINE_POINTER_CALL(0x7360C9, &CanUpdate);


DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	if (!pBuilding || !pBuilding->Owner)
		return 0;

	if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
	{
		auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

		if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
		{
			BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
			amount = 0.0f;
		}
	}

	return 0;
}
