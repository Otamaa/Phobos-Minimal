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

// we hook at the beggining of the function
// ares hooking at the beggining of switch call
DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	REF_STACK(ActionArgs const, args, 0x4);
	enum { return_value = 0x6DD910 , continue_func = 0x0 };

	bool handled;
	if (TActionExt::Occured(pThis, args, handled)) {
		R->AL(handled);
		return return_value;
	}

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

	if (pThis->Param5) {
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
			GameDelete<true,false>(pBld);
			//pBld->UnInit();
		}
		else
		{
			if(!bPlayBuildUp)
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

// Bugfix, #issue 429: Retint map script disables RGB settings on light source
// Author: secsome, Starkku
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	// Flag the light sources to update, actually do it later and only once to prevent redundancy.
	RetintTemp::UpdateLightSources = true;

	return 0;
}

#include <Ext/Terrain/Body.h>

// Update light sources if they have been flagged to be updated.
DEFINE_HOOK(0x6D4455, Tactical_Render_UpdateLightSources, 0x8)
{
	if (RetintTemp::UpdateLightSources)
	{
		for (auto pBld : *BuildingClass::Array)
		{
			if (pBld->LightSource && pBld->LightSource->Activated)
			{
				pBld->LightSource->Activated = false;
				pBld->LightSource->Activate();
			}
		}

		for (auto pRadSite : *RadSiteClass::Array)
		{
			if (pRadSite->LightSource && pRadSite->LightSource->Activated)
			{
				pRadSite->LightSource->Activated = false;
				pRadSite->LightSource->Activate();
			}
		}

		for (auto pTerrain : *TerrainClass::Array)
		{
			auto pExt = TerrainExtContainer::Instance.Find(pTerrain);

			if (pExt->LighSource && pExt->LighSource->Activated)
			{
				pExt->LighSource->Activated = false;
				pExt->LighSource->Activate();
			}
		}

		RetintTemp::UpdateLightSources = false;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x6E0AA0, TActionClass_ChangeHouse_IncludePassengers, 0x7)
{
	GET(TActionClass*, pThis, ECX);
	REF_STACK(ActionArgs const, args, 0x4);

	bool changed = false;
	if (args.pTrigger) {
		auto NewOwner = pThis->Value;

		HouseClass* NewOwnerPtr = nullptr;
		if (NewOwner == 8997)
			NewOwnerPtr = args.pTrigger->House;
		else
		{

			if (NewOwner == -1)
			{
				R->EAX(false);
				return 0x6E0AE7;
			}

			if (HouseClass::Index_IsMP(NewOwner))
			{
				NewOwnerPtr = HouseClass::FindByPlayerAt(NewOwner);
			}
			else
			{
				NewOwnerPtr = HouseClass::FindByCountryIndex(NewOwner);
			}
		}

		if (NewOwnerPtr)
		{
			TechnoClass::Array->for_each([&](TechnoClass* pItem) {
				if (!pItem->IsAlive || pItem->Health <= 0 || !pItem->IsOnMap || pItem->InLimbo)
					return;

				if (pItem->AttachedTag && pItem->AttachedTag->ContainsTrigger(args.pTrigger)) {
					pItem->SetOwningHouse(NewOwnerPtr, false);

					if (pThis->Param3 != 0 && pItem->Passengers.FirstPassenger)
					{
						FootClass* pPassenger = pItem->Passengers.FirstPassenger;

						do
						{
							pPassenger->SetOwningHouse(NewOwnerPtr, false);
							pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
						}
						while (pPassenger != nullptr && pPassenger->Transporter == pItem);
					}

					changed = true;
				}
			});
		}
	}

	R->EAX(changed);
	return 0x6E0AE7;
}

DEFINE_HOOK(0x6E0B60, TActionClass_SwitchAllObjectsToHouse, 0x9)
{
	GET(TActionClass*, pThis, ECX);
	REF_STACK(ActionArgs const, args, 0x4);

	bool changed = false;

	if (args.pTrigger != nullptr)
	{
		auto NewOwner = pThis->Value;

		HouseClass* NewOwnerPtr = nullptr;
		if (NewOwner == 8997)
			NewOwnerPtr = args.pTrigger->House;
		else
		{

			if (NewOwner == -1)
			{
				R->EAX(false);
				return 0x6E0AE7;
			}

			if (HouseClass::Index_IsMP(NewOwner))
			{
				NewOwnerPtr = HouseClass::FindByPlayerAt(NewOwner);
			}
			else
			{
				NewOwnerPtr = HouseClass::FindByCountryIndex(NewOwner);
			}
		}

		if (NewOwnerPtr) {

			TechnoClass::Array->for_each([&](TechnoClass* pItem) {
				if (!pItem->IsAlive || pItem->Health <= 0 || pItem->Owner != args.pHouse || pItem->Owner == NewOwnerPtr)
					 return;

				if (pThis->Param3 && pItem->Passengers.FirstPassenger != nullptr)
				{
					FootClass* pPassenger = pItem->Passengers.FirstPassenger;

					do
					{
						pPassenger->SetOwningHouse(NewOwnerPtr, false);
						pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
					}
					while (pPassenger != nullptr && pPassenger->Transporter == pItem);
				}

				pItem->SetOwningHouse(NewOwnerPtr, false);

				if (BuildingClass* pBuilding = specific_cast<BuildingClass*>(pItem)) {
					if (pBuilding->Type->Powered || pBuilding->Type->PoweredSpecial) {
						pBuilding->UpdatePowerDown();
					}
				}

				changed = true;
			});			
		}
	}

	R->EAX(changed);
	return 0x6E0C91;
}

DEFINE_HOOK(0x6DD614, TActionClass_LoadFromINI_GetActionIndex_ParamAsName, 0x6)
{
	GET(TActionClass*, pThis, EBP);

	if (pThis->ActionKind == TriggerAction::PlayAnimAt) {
		GET(char*, pName, ESI);

		if (GeneralUtils::IsValidString(pName) && (pName[0] < '0' || pName[0] > '9')) {
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
