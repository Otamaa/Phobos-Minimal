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

DEFINE_HOOK(0x7394FF, UnitClass_TryToDeploy_CantDeployVoice, 0x8)
{
	GET(UnitClass* const, pThis, EBP);

	const auto pThisTechno = TechnoTypeExt::ExtMap.Find(pThis->Type);

	VoxClass::Play(GameStrings::EVA_CannotDeployHere());

	if (pThisTechno->VoiceCantDeploy.isset()) {
		VocClass::PlayGlobal(pThisTechno->VoiceCantDeploy, Panning::Center, 1.0);
	}

	return 0x73950F;
}