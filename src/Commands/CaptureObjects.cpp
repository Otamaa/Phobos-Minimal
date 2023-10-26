#include "CaptureObjects.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/Ares/Hooks/AresNetEvent.h>

bool CaptureObjectsCommandClass::Given = false;

const char* CaptureObjectsCommandClass::GetName() const
{
	return "Capture Selected Object(s)";
}

const wchar_t* CaptureObjectsCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CAPTUREOBJECTS", L"Capture Selected Object");
}

const wchar_t* CaptureObjectsCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* CaptureObjectsCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CAPTUREOBJECTS_DESC", L"Take ownership of any selected objects.");
}

void CaptureObjectsCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	if (!Phobos::Otamaa::IsAdmin)
		return;

	if (!ObjectClass::CurrentObjects->Count)
		return;

	ObjectClass::CurrentObjects->for_each([](ObjectClass* const object) {
		if (TechnoClass* techno = generic_cast<TechnoClass*>(object)) {

			auto const pToOwner = HouseClass::CurrentPlayer();

			if (techno->GetOwningHouse() == pToOwner)
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
		//Debug::Log("Giving Money to Player ! \n");
		pHouseExt->CaptureObjectExecuted = true;
	}

	//if (auto pBld = BuildingTypeClass::Find("AIYAREFN"))
	//{
	//	auto pObj = (BuildingClass*)pBld->CreateObject(HouseClass::CurrentPlayer());
	//	auto coord = CellClass::Cell2Coord(WWMouseClass::Instance->GetCellUnderCursor());
	//	pObj->Unlimbo(coord, DirType::North);
	//}
}
