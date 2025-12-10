#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>

//bool __fastcall CanUpdate(UnitClass* pThis, void*) {
//	return pThis->DeathFrameCounter != -1 || pThis->IsDeactivated();
//}

//DEFINE_FUNCTION_JUMP(CALL,0x7360C9, GET_OFFSET(CanUpdate));

// ASMJIT_PATCH(0x73E474, UnitClass_Unload_Storage, 0x6)
// {
// 	GET(BuildingClass* const, pBuilding, EDI);
// 	GET(int const, idxTiberium, EBP);
// 	REF_STACK(float, amount, 0x1C);
//
// 	if (!pBuilding || !pBuilding->Owner)
// 		return 0;
//
// 	auto storageTiberiumIndex = RulesExtData::Instance()->Storage_TiberiumIndex;
//
// 	if (BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Refinery_UseStorage && storageTiberiumIndex >= 0)
// 	{
// 		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
// 		amount = 0.0f;
// 	}
//
// 	return 0;
// }

ASMJIT_PATCH(0x7394FF, UnitClass_TryToDeploy_CantDeployVoice, 0x8)
{
	GET(UnitClass* const, pThis, EBP);

	const auto pThisTechno = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	VoxClass::Play(GameStrings::EVA_CannotDeployHere());

	if (pThisTechno->VoiceCantDeploy.isset()) {
		//pThis->QueueVoice(pThisTechno->VoiceCantDeploy);
		VocClass::SafeImmedietelyPlayAt(pThisTechno->VoiceCantDeploy, &pThis->Location);
	}

	return 0x73950F;
}

ASMJIT_PATCH(0x74159F, UnitClass_ApproachTarget_GoAboveTarget, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	auto pType = pThis->Type;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	R->AL(pType->BalloonHover || pTypeExt->CanGoAboveTarget);
	return R->Origin() + 0x6;
}