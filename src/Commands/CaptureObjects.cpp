#include "CaptureObjects.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Event/Body.h>

bool CaptureObjectsCommandClass::Given = false;

const char* CaptureObjectsCommandClass::GetName() const
{
	return "Capture Selected Object(s)";
}

const wchar_t* CaptureObjectsCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_CAPTUREOBJECTS", L"Capture Selected Object");
}

const wchar_t* CaptureObjectsCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* CaptureObjectsCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_CAPTUREOBJECTS_DESC", L"Take ownership of any selected objects.");
}

void CaptureObjectsCommandClass::Execute(WWKey eInput) const
{
	if (!ObjectClass::CurrentObjects->Count)
		return;

	ObjectClass::CurrentObjects->for_each([](ObjectClass* const object) {
		if (TechnoClass* techno = flag_cast_to<TechnoClass*>(object)) {

			auto const pToOwner = HouseClass::CurrentPlayer();

			if (techno->Owner == pToOwner)
			{
				return;
				//if ((techno->AbstractFlags & AbstractFlags::Foot) && !Is_DriverKilled(techno)){
				//	techno->SetOwningHouse(HouseExtData::FindSpecial(), false);
				//	techno->QueueMission(Mission::Harmless, true);
				//}
			}
			else
			{
				if (TechnoExtContainer::Instance.Find(techno)->Is_DriverKilled)
					TechnoExtContainer::Instance.Find(techno)->Is_DriverKilled = false;

				techno->SetOwningHouse(pToOwner, false);
			}
		}
	});

	auto const pHouseExt = HouseExtContainer::Instance.TryFind(HouseClass::CurrentPlayer());
	if (!pHouseExt)
		return;

	if (!pHouseExt->CaptureObjectExecuted)
	{
		HouseClass::CurrentPlayer()->TransactMoney(100000);
		//Debug::LogInfo("Giving Money to Player ! ");
		pHouseExt->CaptureObjectExecuted = true;
	}

	//if (auto pBld = BuildingTypeClass::Find("AIYAREFN"))
	//{
	//	auto pObj = (BuildingClass*)pBld->CreateObject(HouseClass::CurrentPlayer());
	//	auto coord = CellClass::Cell2Coord(WWMouseClass::Instance->GetCellUnderCursor());
	//	pObj->Unlimbo(coord, DirType::North);
	//}
}
