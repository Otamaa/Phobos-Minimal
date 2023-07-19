#include "RevealMap.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/House/Body.h>
#include <Ext/SWType/NewSuperWeaponType/Reveal.h>

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
#include <Misc/MapRevealer.h>
#include <Misc/Ares/Hooks/AresNetEvent.h>

void RevealMapCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	const auto pPlayer = HouseClass::CurrentPlayer();
	if (!Phobos::Otamaa::IsAdmin || !pPlayer)
		return;

	SW_Reveal::RevealMap(pPlayer->GetBaseCenter(), -1.0f, 0, pPlayer);
	AresNetEvent::Handlers::RaiseRevealMap(pPlayer);
}
