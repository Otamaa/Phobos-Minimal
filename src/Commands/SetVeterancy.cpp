#include "SetVeterancy.h"

#include <Utilities/GeneralUtils.h>

const char* SetVeterancyCommandClass::GetName() const
{
	return "Set Techno Veterancy";
}

const wchar_t* SetVeterancyCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SETTVET", L"Set Techno Veterancy");
}

const wchar_t* SetVeterancyCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* SetVeterancyCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SETTVET_DESC", L"Set Techno Veterancy.");
}

void SetVeterancyCommandClass::Execute(WWKey eInput) const
{
	if (!ObjectClass::CurrentObjects->Count)
		return;

	for (auto pObj : ObjectClass::CurrentObjects())
	{
		const auto pTechn = flag_cast_to<TechnoClass*>(pObj);

		if (!pTechn)
			continue;

		if (pTechn->Veterancy.IsRookie() && !pTechn->Veterancy.IsVeteran() && !pTechn->Veterancy.IsElite())
		{ pTechn->Veterancy.SetVeteran(); }
		else if (!pTechn->Veterancy.IsRookie() && pTechn->Veterancy.IsVeteran() && !pTechn->Veterancy.IsElite())
		{ pTechn->Veterancy.SetElite(); }
	}
}