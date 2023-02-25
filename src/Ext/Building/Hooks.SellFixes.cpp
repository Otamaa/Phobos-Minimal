#include "Body.h"
#include <Utilities/Macro.h>

// Rewrite 0x449BC0
bool MCVCanUndeploy(BuildingClass* pThis)
{
	const auto pType = pThis->Type;

	// Just sell if no UndeploysInto
	if (!pType->UndeploysInto || !pThis->Owner)
		return false;

	if (pThis->Focus && !pThis->MindControlledBy && pThis->Owner->IsControlledByHuman())
	{
		// Canyards can't undeploy if MCVRedeploy=no
		if (pType->ConstructionYard && !GameModeOptionsClass::Instance->MCVRedeploy)
			return false;

		// Can undeploy even if Unsellable=no
		return true;
	}

	return false;
}

// Skip SessionClass::IsCampaign() checks
DEFINE_JUMP(LJMP, 0x443A9A, 0x443AA3) //BuildingClass_SetRallyPoint
DEFINE_JUMP(LJMP, 0x44375E, 0x443767) //BuildingClass_CellClickedAction
DEFINE_JUMP(LJMP, 0x44F602, 0x44F60B) //BuildingClass_IsControllable

DEFINE_HOOK(0x449CC1, BuildingClass_Mission_Destruction_EVASoldAndUndeploysInto, 0x6)
{
	enum { CreateUnit = 0x449D5E, SkipTheEntireShit = 0x44A1E8 };
	GET(BuildingClass*, pThis, EBP);

	if (pThis->IsOwnedByCurrentPlayer && (!pThis->Focus || !pThis->Type->UndeploysInto))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndexById(GameStrings::EVA_StructureSold)));
	}

	return MCVCanUndeploy(pThis) ? CreateUnit : SkipTheEntireShit;
}

DEFINE_HOOK(0x44A7CF, BuildingClass_Mi_Selling_PlaySellSound, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	enum { FinishPlaying = 0x44A85B };

	if (!MCVCanUndeploy(pThis))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	}

	return FinishPlaying;
}

DEFINE_HOOK(0x44A8E5, BuildingClass_Mi_Selling_SetTarget, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	enum { ResetTarget = 0x44A937, SkipShit = 0x44A95E };
	return MCVCanUndeploy(pThis) ? ResetTarget : SkipShit;
}

DEFINE_HOOK(0x44A964, BuildingClass_Mi_Selling_VoiceDeploy, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	enum { CanDeploySound = 0x44A9CA, SkipShit = 0x44AA3D };
	return MCVCanUndeploy(pThis) ? CanDeploySound : SkipShit;
}

DEFINE_JUMP(LJMP, 0x44AB22 ,0x44AB3B) // Structure Sold EVA played twice
