#include "NextIdleHarvester.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>

const char* NextIdleHarvesterCommandClass::GetName() const
{
	return "Next Idle Harvester";
}

const wchar_t* NextIdleHarvesterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_IDLE_HARVESTER", L"Next Idle Harvester");
}

const wchar_t* NextIdleHarvesterCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* NextIdleHarvesterCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_IDLE_HARVESTER_DESC", L"Select the next harvester that is idle (not harvesting).");
}

void NextIdleHarvesterCommandClass::Execute(WWKey eInput) const
{

	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->Items[0] : nullptr);

	bool idleHarvestersPresent = false;
	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto pTechno = flag_cast_to<TechnoClass*>(pNextObject))
		{
			if (auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType()))
			{
				if (pTypeExt->IsCountedAsHarvester() && !TechnoExtData::IsHarvesting(pTechno))
				{
					pObjectToSelect = pNextObject;
					idleHarvestersPresent = true;
					break;
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);

	if (idleHarvestersPresent && pObjectToSelect)
	{
		MapClass::UnselectAll();
		pObjectToSelect->Select();
		MapClass::Instance->CenterMap();
		MapClass::Instance->MarkNeedsRedraw(1);
	} else {
		GeneralUtils::PrintMessage(StringTable::LoadString("MSG:NothingSelected"));
	}
}
