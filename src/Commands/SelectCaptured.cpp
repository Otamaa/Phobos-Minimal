#include "SelectCaptured.h"

#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>
#include <TacticalClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

//https://github.com/Phobos-developers/Phobos/pull/1314

const char* SelectCapturedCommandClass::GetName() const
{
	return "Select Captured Units";
}

const wchar_t* SelectCapturedCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SELECT_CAPTURED", L"Select Captured Units");
}

const wchar_t* SelectCapturedCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* SelectCapturedCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SELECT_CAPTURED_DESC", L"Select the captured units in the current screen.");
}

void SelectCapturedCommandClass::Execute(WWKey eInput) const
{

	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	auto pFirstObject = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	bool capturedPresent = false;
	auto pCurrentObject = pFirstObject;

	do
	{
		if (auto pTechno = flag_cast_to<TechnoClass*>(pCurrentObject))
		{
			if (pTechno->IsOnMyView() && // the unit is in the current screen
				pTechno->IsMindControlled() && pTechno->IsSelectable() && !pTechno->MindControlledByAUnit) // the unit is mc by non-perma mc, and selectable.
			{
				if (!capturedPresent)
				{
					capturedPresent = true;
					MapClass::UnselectAll();
				}

				pCurrentObject->Select();
			}
		}

		pCurrentObject = MapClass::Instance->NextObject(pCurrentObject);
	}
	while (pCurrentObject != pFirstObject);

	auto pMessage = capturedPresent ?
		GeneralUtils::LoadStringUnlessMissingNoChecks("MSG:SelectCaptured", L"Captured units selected.") :
		StringTable::FetchString("MSG:NothingSelected")
		;

	if (capturedPresent) {
		MapClass::Instance->MarkNeedsRedraw(1);
	}

	GeneralUtils::PrintMessage(pMessage);
}