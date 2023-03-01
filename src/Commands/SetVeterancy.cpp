#include "SetVeterancy.h"

#include <Utilities/GeneralUtils.h>

const char* SetVeterancyCommandClass::GetName() const
{
	return "Set Techno Veterancy";
}

const wchar_t* SetVeterancyCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SETTVET", L"Set Techno Veterancy");
}

const wchar_t* SetVeterancyCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* SetVeterancyCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SETTVET_DESC", L"Set Techno Veterancy.");
}

void SetVeterancyCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	if (!ObjectClass::CurrentObjects->Count)
		return;

	for (auto pObj : ObjectClass::CurrentObjects())
	{
		const auto pTechn = generic_cast<TechnoClass*>(pObj);

		if (!pTechn || pTechn->Veterancy.IsElite())
			continue;

		pTechn->Veterancy.SetElite();
	}
}