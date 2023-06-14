#include "RevealMap.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/House/Body.h>

const char* RevealMapCommandClass::GetName() const
{
	return "Reveal Map";
}

const wchar_t* RevealMapCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REVEALMAP", L"Reveal Map");
}

const wchar_t* RevealMapCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* RevealMapCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REVEALMAP_DESC", L"Reveal Map.");
}

#include <Misc/AresData.h>

void RevealMapCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	if (!Phobos::Otamaa::IsAdmin)
		return;

	//Reveal map will causing desync when used on multiplayer
	if (!((SessionClass::Instance->GameMode == GameMode::Campaign) || (SessionClass::Instance->GameMode == GameMode::Skirmish)))
		return;
	
	if(HouseClass::CurrentPlayer())
		MapClass::Instance->Reveal(HouseClass::CurrentPlayer());

}
