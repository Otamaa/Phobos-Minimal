#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

// Rewrite 0x449BC0
bool CanUndeployOnSell(BuildingClass* pThis)
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
		if (!RulesExtData::Instance()->AllowDeployControlledMCV && pThis->MindControlledBy)
			return false;
	}
	//else
	// {
	// 	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);
	// 	if (!pTypeExt->UndeploysInto_Sellable)
	// 		return true;
	// }

	return pThis->ArchiveTarget || pThis->Type->Unsellable;
}

// Skip SessionClass::IsCampaign() checks
DEFINE_JUMP(LJMP, 0x443A9A, 0x443AA3) //BuildingClass_SetRallyPoint
DEFINE_JUMP(LJMP, 0x44375E, 0x443767) //BuildingClass_CellClickedAction
DEFINE_JUMP(LJMP, 0x44F602, 0x44F60B) //BuildingClass_IsControllable
ASMJIT_PATCH(0x700ED0, TechnoClass_AllowDeployControlledMCV, 0x6)// UnitClass::CanDeploySlashUnload
{
	return RulesExtData::Instance()->AllowDeployControlledMCV ? R->Origin() + 0xE : 0;
}ASMJIT_PATCH_AGAIN(0x443770, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::CellClickedAction
ASMJIT_PATCH_AGAIN(0x443AB0, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::SetRallyPoint
ASMJIT_PATCH_AGAIN(0x44F614, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::IsControllable

ASMJIT_PATCH(0x449CC1, BuildingClass_Mission_Destruction_EVASoldAndUndeploysInto, 0x6)
{
	enum { CreateUnit = 0x449D5E, SkipTheEntireShit = 0x44A1E8 };
	GET(BuildingClass*, pThis, EBP);

	if (pThis->IsOwnedByCurrentPlayer &&
		(!pThis->ArchiveTarget || !pThis->Type->UndeploysInto)) {
		VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pThis->Type)->EVA_Sold.Get());
	}

	return CanUndeployOnSell(pThis) ? CreateUnit : SkipTheEntireShit;
	//return 0x449CEA;
}

ASMJIT_PATCH(0x44A827, BuildingClass_Mi_Selling_PlaySellSound, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	if (!CanUndeployOnSell(pThis)) {
		VocClass::SafeImmedietelyPlayAt(TechnoTypeExtContainer::Instance.Find(pThis->Type)->SellSound.Get(), &pThis->Location);
	}

	return 0x44A85B;
}

ASMJIT_PATCH(0x44A8E5, BuildingClass_Mi_Selling_SetTarget, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	enum { ResetTarget = 0x44A937, SkipShit = 0x44A95E };
	return CanUndeployOnSell(pThis) ? ResetTarget : SkipShit;
}

ASMJIT_PATCH(0x44A964, BuildingClass_Mi_Selling_VoiceDeploy, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	enum { CanDeploySound = 0x44A9CA, SkipShit = 0x44AA3D };
	return CanUndeployOnSell(pThis) ? CanDeploySound : SkipShit;
}

DEFINE_JUMP(LJMP, 0x44AB22 ,0x44AB3B) // Structure Sold EVA played twice
