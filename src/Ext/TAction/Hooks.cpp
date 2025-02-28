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
int lastAction;

 DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
 {
 	GET(TActionClass*, pThis, ECX);
 	REF_STACK(ActionArgs const, args, 0x4);
	//GET_STACK(DWORD , caller , 0x0);

 	enum { return_value = 0x6DD910, continue_func = 0x0 };

	if(((int)pThis->ActionKind == 14 || (int)pThis->ActionKind == 32) ){

		if (lastAction == (int)pThis->ActionKind)
			++StaticVars::TriggerCounts[pThis];
		else
			StaticVars::TriggerCounts[pThis] = 0;

	 	//Debug::LogInfo("TAction[%x] triggering [%d] caller[%x]" , pThis , (int)pThis->ActionKind , caller);

		if (StaticVars::TriggerCounts[pThis] > 1000)
			Debug::FatalErrorAndExit("Possible Deadlock Detected From TAction[%x] with Kind[%d] !", pThis, (int)pThis->ActionKind);
	}

	lastAction = (int)pThis->ActionKind;

 	bool handled;
 	if (TActionExt::Occured(pThis, args, handled))
 	{
 		//Debug::LogInfo("TAction[%x] triggering Phobos [%d]" , pThis , (int)pThis->ActionKind);
 		R->AL(handled);
 		return return_value;
 	}

 	//Debug::LogInfo("TAction[%x] triggering vanilla [%d]" , pThis , (int)pThis->ActionKind);

 	return continue_func;
 }

// Bugfix: TAction 125 Build At do not display the buildups
// Author: secsome
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFS(0x24, 0x18));

	if (pThis->Param5)
	{
		if (HouseClass::Index_IsMP(pThis->Param6))
			pHouse = HouseClass::FindByIndex(pThis->Param6);
		else
			pHouse = HouseClass::FindByCountryIndex(pThis->Param6);
	}

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
			GameDelete<true, false>(pBld);
			//pBld->UnInit();
		}
		else
		{
			if (!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;

			if (pThis->Param3 > 1 &&
				SessionClass::Instance->GameMode == GameMode::Campaign &&
				!pHouse->ControlledByCurrentPlayer())
				pBld->ShouldRebuild = true;

			bCreated = true;
		}
	}

	R->AL(bCreated);
	return 0x6E42C1;
}

#pragma region RetintFix

namespace RetintTemp
{
	bool UpdateLightSources = false;
}

// Update light sources if they have been flagged to be updated.
DEFINE_HOOK(0x6D4455, Tactical_Render_UpdateLightSources, 0x8)
{
	if (ScenarioExtData::UpdateLightSources)
	{
		for (auto light : LightSourceClass::Array()){
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

DEFINE_HOOK(0x6E0AA0, TActionClass_ChangeHouse_IncludePassengers, 0x7)
{
	GET(TActionClass*, pThis, ECX);
	REF_STACK(ActionArgs const, args, 0x4);

	bool changed = false;
	if (args.pTrigger)
	{
		if (HouseClass* NewOwnerPtr = AresTEventExt::ResolveHouseParam(pThis->Value, args.pTrigger->GetHouse()))
		{
			for (int i = 0; i < TechnoClass::Array->Count; ++i) {

				const auto pItem = TechnoClass::Array->Items[i];

				if (!pItem)
					continue;

//				Debug::LogInfo("ChangeOwner for [%s] from [%x] with param3 [%d] [ %s(%x) -> %s(%x) ]", pItem->get_ID(), pThis, pThis->Param3, pItem->Owner->get_ID(), pItem->Owner, NewOwnerPtr->get_ID(), NewOwnerPtr);

				if (!pItem->IsAlive || pItem->Health <= 0 || pItem->InLimbo || !pItem->IsOnMap)
					continue;

				if (pItem->AttachedTag && pItem->AttachedTag->ContainsTrigger(args.pTrigger))
				{
					pItem->SetOwningHouse(NewOwnerPtr, false);

					if (pThis->Param3 != 0 && pItem->Passengers.FirstPassenger)
					{
						FootClass* pPassenger = pItem->Passengers.FirstPassenger;

						do
						{
							pPassenger->SetOwningHouse(NewOwnerPtr, false);
							pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
						}
						while (pPassenger != nullptr && pPassenger->Transporter == pItem);
					}

					changed = true;
				}
			}
		}
	}

	R->AL(changed);
	return 0x6E0B50;
}

DEFINE_HOOK(0x6E0B60, TActionClass_SwitchAllObjectsToHouse, 0x9)
{
	GET(TActionClass*, pThis, ECX);
	REF_STACK(ActionArgs const, args, 0x4);

	bool changed = false;

	if (args.pTrigger)
	{
		if (HouseClass* NewOwnerPtr = AresTEventExt::ResolveHouseParam(pThis->Value, args.pTrigger->GetHouse()))
		{
			for (int i = 0; i < TechnoClass::Array->Count; ++i)
			{
				const auto pItem = TechnoClass::Array->Items[i];

				if (!pItem)
					continue;

				if (!pItem->IsAlive || pItem->Health <= 0 || pItem->Owner != args.pHouse)
					continue;

				//Debug::LogInfo("SwitchAllObjectsToHouse for [%s] from [%x] with param3 [%d] [ %s -> %s ]", pItem->get_ID(), pThis, pThis->Param3, pItem->Owner->get_ID(), NewOwnerPtr->get_ID());

				if (pThis->Param3 && pItem->Passengers.FirstPassenger != nullptr)
				{
					FootClass* pPassenger = pItem->Passengers.FirstPassenger;

					do
					{
						pPassenger->SetOwningHouse(NewOwnerPtr, false);
						pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
					}
					while (pPassenger != nullptr && pPassenger->Transporter == pItem);
				}

				pItem->SetOwningHouse(NewOwnerPtr, false);

				if (BuildingClass* pBuilding = cast_to<BuildingClass*, false>(pItem))
				{
					if (pBuilding->Type->Powered || pBuilding->Type->PoweredSpecial)
					{
						pBuilding->UpdatePowerDown();
					}
				}

				changed = true;
			}
		}
	}

	R->EAX(changed);
	return 0x6E0C91;
}

DEFINE_HOOK(0x6DD614, TActionClass_LoadFromINI_GetActionIndex_ParamAsName, 0x6)
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

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome
//DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
//DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
//DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
//{
//	if (ScenarioExtData::Instance()->AdjustLightingFix)
//		TActionExt::RecreateLightSources();
//
//	const TintStruct tint = ScenarioClass::Instance->NormalLighting.Tint;
//	ScenarioExtData::Instance()->CurrentTint_Tiles = tint;
//	ScenarioExtData::Instance()->CurrentTint_Schemes = tint;
//	ScenarioExtData::Instance()->CurrentTint_Hashes = tint;
//
//	return 0;
//}

//
//DEFINE_HOOK(0x6E0D60, TActionClass_Text_Trigger, 0x6)
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
