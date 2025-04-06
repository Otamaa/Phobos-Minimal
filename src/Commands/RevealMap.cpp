#include "RevealMap.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/House/Body.h>
#include <Ext/SWType/NewSuperWeaponType/Reveal.h>
#include <Utilities/GeneralUtils.h>
#include <EASTL/vector.h>

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
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* RevealMapCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REVEALMAP_DESC", L"Reveal Map.");
}

#include <Misc/MapRevealer.h>
#include <Ext/Event/Body.h>

static eastl::vector<eastl::vector<TechnoClass*>> DumpedTechno;

void NOINLINE ClearDumped()
{
	DumpedTechno.clear();
	DumpedTechno.resize(HouseClass::Array->Count);
}

void NOINLINE GrupDumped()
{
	for (size_t i = 0; i < DumpedTechno.size(); ++i)
	{
		for (auto pTechn : *TechnoClass::Array)
		{
			if (pTechn->Owner == HouseClass::Array->Items[i])
			{
				DumpedTechno[i].push_back(pTechn);
			}
		}
	}
}

void NOINLINE DumpDumped()
{
	for (size_t i = 0; i < DumpedTechno.size(); ++i)
	{
		Debug::LogInfo("Dumping Techno  For[{}]", HouseClass::Array->Items[i]->Type->ID);
		for (auto const& data : DumpedTechno[i])
		{
			Debug::LogInfo("Techno [{}]", data->get_ID());
		}
	}
}

void RevealMapCommandClass::Execute(WWKey eInput) const
{
	const auto pPlayer = HouseClass::CurrentPlayer();
	if (!pPlayer)
		return;

	SW_Reveal::RevealMap(pPlayer->GetBaseCenter(), -1.0f, 0, pPlayer);

	//ClearDumped();
	//GrupDumped();
	//DumpDumped();

	//if(SessionClass::Instance->GameMode == GameMode::Internet || SessionClass::Instance->GameMode == GameMode::LAN )
		//EventExt::Handlers::RaiseRevealMap(pPlayer);
}
