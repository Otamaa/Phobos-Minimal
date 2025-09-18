#include "CeasefireModeClass.h"

#include <Utilities/GeneralUtils.h>

#include <Ext/Techno/Body.h>
#include <Ext/Event/Body.h>
#include <InfantryClass.h>

const char* CeasefireModeClass::GetName() const
{
	return "CeasefireMode";
}

const wchar_t* CeasefireModeClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_MODE", L"Ceasefire Mode");
}

const wchar_t* CeasefireModeClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* CeasefireModeClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CEASEFIRE_MODE_DESC", L"Ceasefire Mode");
}

void CeasefireModeClass::Execute(WWKey eInput) const
{
	static std::set<TechnoClass*> TechnoVectorCeasefire;
	static std::set<TechnoClass*> TechnoVectorNonCeasefire;

	TechnoVectorCeasefire.clear();
	TechnoVectorNonCeasefire.clear();

	// Get current selected units.
	// If all selected units are at Ceasefire mode, we should cancel their Ceasefire mode.
	// Otherwise, we should turn them into Ceasefire mode.
	bool isAnySelectedUnitTogglable = false;
	bool isAllSelectedUnitCeasefireMode = true;

	auto processATechno = [&](TechnoClass* pTechno)
		{
			const auto pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

			// If not togglable then exclude it from the iteration.
			if (!pTechnoExt->CanTogglePassiveAcquireMode())
				return;

			isAnySelectedUnitTogglable = true;

			if (pTechnoExt->GetPassiveAcquireMode() == PassiveAcquireMode::Ceasefire)
			{
				TechnoVectorCeasefire.emplace(pTechno);
			}
			else
			{
				isAllSelectedUnitCeasefireMode = false;
				TechnoVectorNonCeasefire.emplace(pTechno);
			}
			return;
		};

	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = flag_cast_to<TechnoClass*>(pUnit);

		// if not a techno or is in berserk or is not controlled by the local player then ignore it
		if (!pTechno || pTechno->Berzerk || !pTechno->Owner->IsControlledByHuman())
			continue;

		processATechno(pTechno);

		if (auto pPassenger = pTechno->Passengers.GetFirstPassenger())
		{
			for (; pPassenger; pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject))
				processATechno(pPassenger);
		}

		if (auto pBuilding = cast_to<BuildingClass*>(pTechno))
		{
			for (auto pOccupier : pBuilding->Occupants)
				processATechno(pOccupier);
		}
	}

	// If this boolean is false, then none of the selected units are togglable, meaning this hotket doesn't need to do anything.
	if (isAnySelectedUnitTogglable)
	{
		// If all selected units are Ceasefire mode, then cancel their Ceasefire mode;
		// otherwise, make all selected units Ceasefire mode.
		if (isAllSelectedUnitCeasefireMode)
		{
			for (const auto& pTechno : TechnoVectorCeasefire)
				EventExt::TogglePassiveAcquireMode::Raise(pTechno, PassiveAcquireMode::Normal);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CEASEFIRE_MODE_OFF", L"%i unit(s) ceased Ceasefire Mode."), TechnoVectorCeasefire.size());
			MessageListClass::Instance->PrintMessage(buffer);
		}
		else
		{
			for (const auto& pTechno : TechnoVectorNonCeasefire)
				EventExt::TogglePassiveAcquireMode::Raise(pTechno, PassiveAcquireMode::Ceasefire);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:CEASEFIRE_MODE_ON", L"%i unit(s) entered Ceasefire Mode."), TechnoVectorNonCeasefire.size());
			MessageListClass::Instance->PrintMessage(buffer);
		}
	}
}