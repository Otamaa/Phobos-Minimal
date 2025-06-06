#include "DamageDisplay.h"

#include <Utilities/GeneralUtils.h>

const char* DamageDisplayCommandClass::GetName() const
{
	return "Display Damage Numbers";
}

const wchar_t* DamageDisplayCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DISPLAY_DAMAGE", L"Display Damage Dealt");
}

const wchar_t* DamageDisplayCommandClass::GetUICategory() const
{
	return CATEGORY_GUIDEBUG;
}

const wchar_t* DamageDisplayCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DISPLAY_DAMAGE_DESC", L"Display exact number of damage dealt to units & buildings on them.");
}

void DamageDisplayCommandClass::Execute(WWKey eInput) const
{
	Phobos::Debug_DisplayDamageNumbers = !Phobos::Debug_DisplayDamageNumbers;
}