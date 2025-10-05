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
#include <Misc/Ares/Hooks/Header.h>

// we hook at the beggining of the function
// ares hooking at the beggining of switch call
// int lastAction;

//  ASMJIT_PATCH(0x6DD8B0, TActionClass_Execute, 0x6)
//  {
//  	GET(TActionClass*, pThis, ECX);
//  	REF_STACK(ActionArgs const, args, 0x4);
// 	//GET_STACK(DWORD , caller , 0x0);

//  	enum { return_value = 0x6DD910, continue_func = 0x0 };

// 	if(((int)pThis->ActionKind == 14 || (int)pThis->ActionKind == 32) ){

// 		if (lastAction == (int)pThis->ActionKind)
// 			++StaticVars::TriggerCounts[pThis];
// 		else
// 			StaticVars::TriggerCounts[pThis] = 0;

// 	 	//Debug::LogInfo("TAction[%x] triggering [%d] caller[%x]" , pThis , (int)pThis->ActionKind , caller);

// 		if (StaticVars::TriggerCounts[pThis] > 1000)
// 			Debug::FatalErrorAndExit("Possible Deadlock Detected From TAction[%x] with Kind[%d] !", pThis, (int)pThis->ActionKind);
// 	}

// 	lastAction = (int)pThis->ActionKind;



//  	//Debug::LogInfo("TAction[%x] triggering vanilla [%d]" , pThis , (int)pThis->ActionKind);

//  	return continue_func;
//  }

#pragma region RetintFix

namespace RetintTemp
{
	bool UpdateLightSources = false;
}

// Update light sources if they have been flagged to be updated.
ASMJIT_PATCH(0x6D4455, Tactical_Render_UpdateLightSources, 0x8)
{
	if (ScenarioExtData::UpdateLightSources)
	{
		for (auto light : *LightSourceClass::Array){
			if (light->Activated) {
				light->Activated = false;
				light->Activate();
			}
		}

		ScenarioExtData::UpdateLightSources = false;
	}

	return 0;
}

#pragma endregion
#include <Misc/Ares/Hooks/Header.h>
#include <TriggerTypeClass.h>

ASMJIT_PATCH(0x6DD614, TActionClass_LoadFromINI_GetActionIndex_ParamAsName, 0x6)
{
	GET(TActionClass*, pThis, EBP);

	if (pThis->ActionKind == TriggerAction::PlayAnimAt)
	{
		GET(char*, pName, ESI);

		if (GeneralUtils::IsValidString(pName) && (pName[0] < '0' || pName[0] > '9'))
		{
			const int idx = AnimTypeClass::FindIndexById(pName);

			if (idx >= 0)
				R->EDX(idx);
		}
	}

	return 0;
}

// add/subtract extra values to prevent the AI from attacking the wrong target during the campaign.
/*
ASMJIT_PATCH(0x6DE189, TActionClass_MakeEnemy, 0x6)
{
	GET(TActionClass*, pThis, ESI);
	GET_STACK(HouseClass*, pHouse, STACK_OFFSET(0x250, 0x4));
	GET_STACK(TriggerClass*, pTrigger, STACK_OFFSET(0x250, 0xC));

	enum { OK = 0x6DE1CD, Cancel = 0x6DE1A5 };

	HouseClass* pTargetHouse = pThis->FindHouseByIndex(pTrigger, pThis->Value);

	if (!pHouse || !pTargetHouse || pHouse == pTargetHouse)
		return Cancel;

	pHouse->MakeEnemy(pTargetHouse, false);
	pTargetHouse->MakeEnemy(pHouse, false);

	// Maybe there's a better way, but I want to make it simple.
	if (SessionClass::Instance->IsCampaign() || (!pHouse->Type->MultiplayPassive && !pTargetHouse->Type->MultiplayPassive)) {

		if (pThis->Param3 != 0)
			pHouse->UpdateAngerNodes(pThis->Param3, pTargetHouse);

		if (pThis->Param3 < 0 && pHouse->AngerNodes.Count > 0) {
			for (auto& pAngerNode : pHouse->AngerNodes) {

				if (pAngerNode.House != pTargetHouse)
					continue;

				pAngerNode.AngerLevel = MaxImpl(0, pAngerNode.AngerLevel);
				break;
			}
		}
	}

	return OK;
}*/

#include <Ext/Rules/Body.h>

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome, Starkku

ASMJIT_PATCH(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Flag the light sources to update, actually do it later and only once to prevent redundancy.
	RetintTemp::UpdateLightSources = RulesExtData::Instance()->UseRetintFix;

	return 0;
}ASMJIT_PATCH_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
ASMJIT_PATCH_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
//	ScenarioExtData::Instance()->CurrentTint_Schemes = tint;
//	ScenarioExtData::Instance()->CurrentTint_Hashes = tint;
//
//	return 0;
//}

//
//ASMJIT_PATCH(0x6E0D60, TActionClass_Text_Trigger, 0x6)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pHouse, 0x4);
//	GET_STACK(TriggerClass*, pTrigger, 0xC);
//
//	int nNewOwner = pThis->Value;
//	HouseClass* pNewOwner = nullptr;
//
//	if (nNewOwner == 0x2325)
//		pNewOwner = pTrigger->GetHouse();
//	else
//	{
//		if (nNewOwner == -1)
//			return 0;
//		if (pHouse->Index_IsMP(nNewOwner))
//			pNewOwner = pHouse->FindByIndex(nNewOwner);
//		else
//			pNewOwner = pHouse->FindByCountryIndex(nNewOwner);
//	}
//
//	if (!pNewOwner || HouseClass::CurrentPlayer == pNewOwner)
//		return 0;
//
//	R->AL(1);
//	return 0x6E0DE5;
//}
