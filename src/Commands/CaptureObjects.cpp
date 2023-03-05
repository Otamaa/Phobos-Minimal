#include "CaptureObjects.h"

#include <SessionClass.h>
#include <HouseClass.h>

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

	std::for_each(ObjectClass::CurrentObjects->begin(), ObjectClass::CurrentObjects->end(), [](ObjectClass* const object)
{
	if (!object || !(object->AbstractFlags & AbstractFlags::Techno))
		return;

	if (object->GetOwningHouse() == HouseClass::CurrentPlayer())
		return;

	if (TechnoClass* techno = static_cast<TechnoClass*>(object))
		techno->SetOwningHouse(HouseClass::CurrentPlayer());

	});

	if (!Given)
	{
		HouseClass::CurrentPlayer()->TransactMoney(10000000);
		Given = true;
	}

	if (!HouseClass::CurrentPlayer->Visionary)
	{
		HouseClass::CurrentPlayer->Visionary = 1;
		MapClass::Instance->CellIteratorReset();
		for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
			RadarClass::Instance->MapCell(i->MapCoords, HouseClass::CurrentPlayer);

		GScreenClass::Instance->MarkNeedsRedraw(1);
	}
}
