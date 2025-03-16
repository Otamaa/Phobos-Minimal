#include <Ext/Building/Body.h>

#include <Ext/BuildingType/Body.h>
#include <SidebarClass.h>
#include <InfantryClass.h> // The easiest way to get BuildingClass pass compile
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <MouseClass.h>

// #include <windowsx.h> // for mouse related macros
// vvv FactoryBuildingMe vvv
ASMJIT_PATCH(0x4C9DA2, FactoryClass_Set_BuildingSetFactoryBuildingMe, 0x5)
{
	GET(FactoryClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EAX);
	if (pObject->WhatAmI() == AbstractType::Building)
	{
		const auto pBld = static_cast<BuildingClass*>(pObject);
		const auto pExt = BuildingExtContainer::Instance.Find(pBld);

		pExt->FactoryBuildingMe = pThis;
	}
	return 0;
}

ASMJIT_PATCH(0x4CA004, FactoryClass_Abandon_BuildingUnsetFactoryBuildingMe, 0x9)
{
	GET(ObjectClass*, pObject, ECX);
	if (pObject->WhatAmI() == AbstractType::Building)
	{
		const auto pBld = static_cast<BuildingClass*>(pObject);
		const auto pExt = BuildingExtContainer::Instance.Find(pBld);
		pExt->FactoryBuildingMe = nullptr;
	}
	return 0;
}

// ^^^ FactoryBuildingMe ^^^
static bool HandleBuildingLink(bool bUp)
{
	const auto pBldType = SidebarClass::Instance->CurrentBuildingType;
	if (!pBldType)
		return false;

	auto pBld = specific_cast<BuildingClass*>(SidebarClass::Instance->CurrentBuilding);

	if (!pBld)
		return false;

	auto pBldExt = 	BuildingExtContainer::Instance.Find(pBld);
	const auto pBldTypeExt = pBldExt->Type;
	const auto pNextBldType = bUp ? pBldTypeExt->NextBuilding_Prev : pBldTypeExt->NextBuilding_Next;
	if (!pNextBldType)
		return false;
	// destruct the old building
	const auto pFactory = 		BuildingExtContainer::Instance.Find(pBld)->FactoryBuildingMe;
	reference<BuildingClass*, 0xB0FE5C> SidebarBuildingTabObject;
	const bool bNeedSetSidebar = SidebarBuildingTabObject == pBld;
	GameDelete(pBld);
	// create the new building, remember to set the FactoryBuildingMe in Ext!
	pBld = reinterpret_cast<BuildingClass*>(pNextBldType->CreateObject(pFactory->Owner));
	pFactory->Object = pBld;
	pBldExt = 		BuildingExtContainer::Instance.Find(pBld);
	pBldExt->FactoryBuildingMe = pFactory;
	SidebarClass::Instance->CurrentBuilding = pBld;
	SidebarClass::Instance->CurrentBuildingType = pNextBldType;
	SidebarClass::Instance->SetActiveFoundation(pBld->GetFoundationData(true));
	if (bNeedSetSidebar)
		SidebarBuildingTabObject = pBld;
	pBldTypeExt->NextBuilding_CurrentHeapId = pNextBldType->ArrayIndex;
	return true;
}
ASMJIT_PATCH(0x777985, Main_Window_Proc_OnMouseWheel, 0x6)
{
	// Singlethread, no need to worry about it, westwood!
	// GET_STACK(HWND, hWnd, STACK_OFFSET(0x10, 0x4));
	// GET_STACK(UINT, uMsg, STACK_OFFSET(0x10, 0x8)); WM_MOUSEWHEEL
	GET_STACK(WPARAM, wParam, STACK_OFFSET(0x10, 0xC));
	// GET_STACK(LPARAM, lParam, STACK_OFFSET(0x10, 0x10));
	const auto fwKeys = GET_KEYSTATE_WPARAM(wParam);
	const auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	// const auto xPos = GET_X_LPARAM(lParam);
	// const auto yPos = GET_Y_LPARAM(lParam);
	// Check if there's pending building placement
	// If there's pending building placement and it have a linked building, then handle it
	const bool bUp = zDelta > 0;
	if (!HandleBuildingLink(bUp))
		SidebarClass::Instance->Scroll(bUp, -1);
	return 0x7779B5;
}
ASMJIT_PATCH(0x4FB8E4, Manual_Place_ResetBuildingTypeCurrentHeapId, 0x6)
{
	if (const auto pType = cast_to<BuildingTypeClass*>(SidebarClass::Instance->CurrentBuildingType))
	{
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);
		pTypeExt->NextBuilding_CurrentHeapId = pType->ArrayIndex;
	}
	return 0;
}
static std::vector<bool> aBuildingConnected;
ASMJIT_PATCH(0x679B9B, RulesClass_Objects_BuildingTypesAfterReadFromINI, 0x0)
{
	// use a dynamic bitset(vector<bool>) to record the processed building types
	aBuildingConnected.resize(BuildingTypeClass::Array->Count, false);

	// We require the INIer write the NextBuilding.Next correctly at least, so we can fix the NextBuilding.Prev for them
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		if (pExt->NextBuilding_Next) {
			const auto pNextExt = BuildingTypeExtContainer::Instance.Find(pExt->NextBuilding_Next);
			if (pNextExt->NextBuilding_Prev == nullptr)
				pNextExt->NextBuilding_Prev = pBldType;
		}
		else
			aBuildingConnected[pBldType->ArrayIndex] = true;
	}

	// Pre connect those already connected ones
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		if (aBuildingConnected[pBldType->ArrayIndex])
			continue;

		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		auto pNextBldType = pExt->NextBuilding_Next;
		while (true)
		{
			if (pNextBldType == nullptr)
				break;

			if (pNextBldType == pBldType) {
				aBuildingConnected[pBldType->ArrayIndex] = true;
				break;
			}

			aBuildingConnected[pNextBldType->ArrayIndex] = true;
			const auto pNextExt = BuildingTypeExtContainer::Instance.Find(pNextBldType);
			pNextBldType = pNextExt->NextBuilding_Next;
		}
	}

	// Check whether there's unmatched NextBuilding_Prev and NextBuilding_Next
	for (auto pBldType : *BuildingTypeClass::Array) {
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		if (const auto pNextBldType = pExt->NextBuilding_Next) {
			const auto pNextExt = BuildingTypeExtContainer::Instance.Find(pNextBldType);
			if (pNextExt->NextBuilding_Prev != pBldType)
				Debug::FatalErrorAndExit("Invalid NextBuilding.Prev for {} and {}", pBldType->ID, pNextBldType->ID);
		}
	}

	// Now fix up the loop automatically
	for (auto pBldType : *BuildingTypeClass::Array)
	{
		if (aBuildingConnected[pBldType->ArrayIndex])
			continue;

		aBuildingConnected[pBldType->ArrayIndex] = true;
		auto pHeadBldType = pBldType;
		while (true)
		{
			const auto pExt = BuildingTypeExtContainer::Instance.Find(pHeadBldType);
			if (!pExt->NextBuilding_Prev)
				break;

			pHeadBldType = pExt->NextBuilding_Prev;
			aBuildingConnected[pHeadBldType->ArrayIndex] = true;
		}
		auto pTailBldType = pBldType;
		while (true)
		{
			const auto pExt = BuildingTypeExtContainer::Instance.Find(pTailBldType);
			if (!pExt->NextBuilding_Next)
				break;

			pTailBldType = pExt->NextBuilding_Next;
			aBuildingConnected[pTailBldType->ArrayIndex] = true;
		}

		const auto pHeadExt = BuildingTypeExtContainer::Instance.Find(pHeadBldType);
		const auto pTailExt = BuildingTypeExtContainer::Instance.Find(pTailBldType);
		pHeadExt->NextBuilding_Prev = pTailBldType;
		pTailExt->NextBuilding_Next = pHeadBldType;
	}

	aBuildingConnected.clear();
	return 0x679BBE;
}
ASMJIT_PATCH(0x6AAFD0, SidebarClass_StripClass_SelectClass_Action_Cancelled_NextBuilding_E_ABANDON, 0x5)
{
	GET(unsigned int, heapid, ECX);
	GET(AbstractType, rttiid, EBP);
	if (rttiid == AbstractType::BuildingType)
	{
		const auto pBldType = BuildingTypeClass::Array->GetItem(heapid);
		if (const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType))
			R->ECX(pTypeExt->NextBuilding_CurrentHeapId);
	}
	return 0;
}

ASMJIT_PATCH(0x6AAE5C, SidebarClass_StripClass_SelectClass_Action_Cancelled_NextBuilding_E_ABANDONALL, 0x5)
{
	GET(unsigned int, heapid, EDX);
	GET(AbstractType, rttiid, EBP);
	if (rttiid == AbstractType::BuildingType) {
		const auto pBldType = BuildingTypeClass::Array->GetItem(heapid);
		if (const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType))
			R->EDX(pTypeExt->NextBuilding_CurrentHeapId);
	}
	return 0;
}