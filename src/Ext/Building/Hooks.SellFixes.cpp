#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

// Rewrite 0x449BC0
bool BuildingCanUnload(BuildingClass* pThis)
{
	auto pType = pThis->Type;

	if (!pType->UndeploysInto)
		return false;

	if (pType->ConstructionYard)
	{
		// Conyards can't undeploy if MCVRedeploy=no
		if (!GameModeOptionsClass::Instance->MCVRedeploy)
			return false;

		// or MindControlledBy YURIX (why? for balance?)
		if (pThis->MindControlledBy || !pThis->Owner->IsControlledByHuman())
			return false;
	}

	return pThis->Focus;
}

// Skip SessionClass::IsCampaign() checks
DEFINE_JUMP(LJMP, 0x443A9A, 0x443AA3) //BuildingClass_SetRallyPoint
DEFINE_JUMP(LJMP, 0x44375E, 0x443767) //BuildingClass_CellClickedAction
DEFINE_JUMP(LJMP, 0x44F602, 0x44F60B) //BuildingClass_IsControllable

DEFINE_HOOK(0x449CC1, BuildingClass_Mission_Destruction_EVASoldAndUndeploysInto, 0x6)
{
	enum { CreateUnit = 0x449D5E, SkipTheEntireShit = 0x44A1E8 };
	GET(BuildingClass*, pThis, EBP);

	if (pThis->IsOwnedByCurrentPlayer &&
		(!pThis->Focus || !pThis->Type->UndeploysInto)) {
		VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pThis->Type)->EVA_Sold.Get());
	}

	//return BuildingCanUnload(pThis) ? CreateUnit : SkipTheEntireShit;
	return 0x449CEA;
}

DEFINE_HOOK(0x44A827, BuildingClass_Mi_Selling_PlaySellSound, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	//if (!BuildingCanUnload(pThis)) {
		VocClass::PlayIndexAtPos(TechnoTypeExtContainer::Instance.Find(pThis->Type)->SellSound.Get(), pThis->Location);
	//}

	return 0x44A85B;
}

//DEFINE_HOOK(0x44A8E5, BuildingClass_Mi_Selling_SetTarget, 0x6)
//{
//	GET(BuildingClass*, pThis, EBP);
//	enum { ResetTarget = 0x44A937, SkipShit = 0x44A95E };
//	return BuildingCanUnload(pThis) ? ResetTarget : SkipShit;
//}

//DEFINE_HOOK(0x44A964, BuildingClass_Mi_Selling_VoiceDeploy, 0x6)
//{
//	GET(BuildingClass*, pThis, EBP);
//	enum { CanDeploySound = 0x44A9CA, SkipShit = 0x44AA3D };
//	return BuildingCanUnload(pThis) ? CanDeploySound : SkipShit;
//}

DEFINE_JUMP(LJMP, 0x44AB22 ,0x44AB3B) // Structure Sold EVA played twice
