#pragma once

#include <Utilities/GeneralUtils.h>
#include <SessionClass.h>
#include <HouseClass.h>

bool Given = false;

class CaptureObjectsCommandClass : public PhobosCommandClass
{
public:

	CaptureObjectsCommandClass() : PhobosCommandClass() { IsDeveloper = true; }
	virtual ~CaptureObjectsCommandClass() { }

	// CommandClass
	virtual const char* GetName() const override
	{
		return "Capture Selected Object(s)";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CAPTUREOBJECTS", L"Capture Selected Object");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CAPTUREOBJECTS_DESC", L"Take ownership of any selected objects.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (!Phobos::Otamaa::IsAdmin)
			return;

		if (!((SessionGlobal.GameMode == GameMode::Campaign) || (SessionGlobal.GameMode == GameMode::Skirmish)))
			return;

		if (!ObjectClass::CurrentObjects)
			return;

		std::for_each(ObjectClass::CurrentObjects->begin(), ObjectClass::CurrentObjects->end(), [](ObjectClass* const object) {
			if (!object || !(object->AbstractFlags & AbstractFlags::Techno))
				return;

			if (object->GetOwningHouse() == HouseClass::CurrentPlayer())
				return;

			if(TechnoClass* techno = abstract_cast<TechnoClass*>(object))
				techno->SetOwningHouse(HouseClass::CurrentPlayer());

		});

		if (!Given) {
			HouseClass::CurrentPlayer()->TransactMoney(10000000);
			Given = true;
		}

		if (!HouseClass::CurrentPlayer->Visionary)
		{
			HouseClass::CurrentPlayer->Visionary = 1;
			MapClass::Instance->CellIteratorReset();
			for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
				RadarClass::Instance->MapCell(&i->MapCoords, HouseClass::CurrentPlayer);

			GScreenClass::Instance->MarkNeedsRedraw(1);
		}
	}
};
