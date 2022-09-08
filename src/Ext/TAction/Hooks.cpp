#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <OverlayTypeClass.h>
#include <LightSourceClass.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <ScenarioClass.h>
#include <TriggerClass.h>

#include <Ext/Scenario/Body.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(ObjectClass*, pObject, 0x8);
	GET_STACK(TriggerClass*, pTrigger, 0xC);
	GET_STACK(CellStruct const*, pLocation, 0x10);

	bool handled;

	R->AL(TActionExt::Execute(pThis, pHouse, pObject, pTrigger, *pLocation, handled));

	return handled ? 0x6DD910 : 0;
}

// Bugfix: TAction 125 Build At do not display the buildups
// Author: secsome
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFS(0x24, 0x18));

	bool bPlayBuildUp = pThis->Param3;

	bool bCreated = false;
	if (auto pBld = static_cast<BuildingClass*>(pBldType->CreateObject(pHouse)))
	{
		if (bPlayBuildUp)
		{
			pBld->BeginMode(BStateType::Construction);
			pBld->QueueMission(Mission::Construction, false);
		}
		else
		{
			pBld->BeginMode(BStateType::Idle);
			pBld->QueueMission(Mission::Guard, false);
		}

		if (!pBld->ForceCreate(coord))
		{
			pBld->UnInit();
		}
		else
		{
			if(!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;

			if (pThis->Param3 > 1 && SessionClass::Instance->GameMode == GameMode::Campaign && !pHouse->IsPlayerControl())
				pBld->ShouldRebuild = true;

			bCreated = true;
		}
	}

	R->AL(bCreated);
	return 0x6E42C1;
}

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	TActionExt::RecreateLightSources();

	ScenarioExt::Global()->CurrentTint_Tiles =
	ScenarioExt::Global()->CurrentTint_Schemes =
	ScenarioExt::Global()->CurrentTint_Hashes = ScenarioClass::Instance->NormalLighting.Tint;

	return 0;
}

DEFINE_HOOK(0x6E0D60, TActionClass_Text_Trigger, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(TriggerClass*, pTrigger, 0xC);

	int nNewOwner = pThis->Value;
	HouseClass* pNewOwner = nullptr;

	if (nNewOwner == 0x2325)
		pNewOwner = pTrigger->GetHouse();
	else
	{
		if (nNewOwner == -1)
			return 0;
		if (pHouse->Index_IsMP(nNewOwner))
			pNewOwner = pHouse->FindByIndex(nNewOwner);
		else
			pNewOwner = pHouse->FindByCountryIndex(nNewOwner);
	}

	if (!pNewOwner || HouseClass::Player == pNewOwner)
		return 0;

	R->AL(1);
	return 0x6E0DE5;
}
