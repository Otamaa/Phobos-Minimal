#include "CaptureObjects.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>

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
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
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

	if (!((SessionClass::Instance->GameMode == GameMode::Campaign) || (SessionClass::Instance->GameMode == GameMode::Skirmish)))
		return;

	if (!ObjectClass::CurrentObjects->Count)
		return;

	std::for_each(ObjectClass::CurrentObjects->begin(), ObjectClass::CurrentObjects->end(), [](ObjectClass* const object) {
		if (!object || !(object->AbstractFlags & AbstractFlags::Techno))
			return;

		auto const pToOwner = HouseClass::CurrentPlayer();
		if (object->GetOwningHouse() == pToOwner)
			return;

		if (TechnoClass* techno = static_cast<TechnoClass*>(object))
			techno->SetOwningHouse(pToOwner);

	});

	auto const pHouseExt = HouseExt::ExtMap.TryFind(HouseClass::CurrentPlayer());
	if (!pHouseExt)
		return;

	if (!pHouseExt->CaptureObjectExecuted)
	{
		HouseClass::CurrentPlayer()->TransactMoney(100000);
		//Debug::Log("Giving Money to Player ! \n");
		pHouseExt->CaptureObjectExecuted = true;
	}

	//HouseClass::CurrentPlayer()->Visionary = true;

	//Debug::Log("Giving RevealMap to Player Start ! \n");
	//static_assert(offsetof(HouseClass, PlanningPaths) == 0x210, "HouseClass ClassMember Shifted !");
	//static_assert(offsetof(HouseClass, Visionary) == 0x240, "HouseClass ClassMember Shifted !");
	//static_assert(offsetof(HouseClass, MapIsClear) == 0x241, "HouseClass ClassMember Shifted !");
	MapClass::Instance->Reveal(HouseClass::CurrentPlayer());
	//Debug::Log("Giving RevealMap to Player Done ! \n");
	//if (!HouseClass::CurrentPlayer->Visionary)
	//{
	//	HouseClass::CurrentPlayer->Visionary = 1;
	//	MapClass::Instance->CellIteratorReset();
	//	for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
	//		RadarClass::Instance->MapCell(i->MapCoords, HouseClass::CurrentPlayer());

	//	GScreenClass::Instance->MarkNeedsRedraw(1);
	//}
}
