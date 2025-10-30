#include "AggressiveModeClass.h"

#include <Utilities/GeneralUtils.h>

#include <Ext/Techno/Body.h>
#include <Ext/Event/Body.h>

#include <InfantryClass.h>

const char* AggressiveModeClass::GetName() const
{
	return "AggressiveMode";
}

const wchar_t* AggressiveModeClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AGGRESSIVE_MODE", L"Aggressive Mode");
}

const wchar_t* AggressiveModeClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AggressiveModeClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AGGRESSIVE_MODE_DESC", L"Aggressive Mode");
}

void AggressiveModeClass::Execute(WWKey eInput) const
{
	static std::set<TechnoClass*> TechnoVectorAggressive;
	static std::set<TechnoClass*> TechnoVectorNonAggressive;

	TechnoVectorAggressive.clear();
	TechnoVectorNonAggressive.clear();

	// Get current selected units.
	// If all selected units are at Aggressive mode, we should cancel their Aggressive mode.
	// Otherwise, we should turn them into Aggressive mode.
	bool isAnySelectedUnitTogglable = false;
	bool isAllSelectedUnitAggressiveMode = true;

	auto processATechno = [&](TechnoClass* pTechno)
		{
			const auto pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

			// If not togglable then exclude it from the iteration.
			if (!pTechnoExt->CanTogglePassiveAcquireMode())
				return;

			isAnySelectedUnitTogglable = true;

			if (pTechnoExt->GetPassiveAcquireMode() == PassiveAcquireMode::Aggressive)
			{
				TechnoVectorAggressive.emplace(pTechno);
			}
			else
			{
				isAllSelectedUnitAggressiveMode = false;
				TechnoVectorNonAggressive.emplace(pTechno);
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
		// If all selected units are Aggressive mode, then cancel their Aggressive mode;
		// otherwise, make all selected units Aggressive mode.
		if (isAllSelectedUnitAggressiveMode)
		{
			for (const auto& pTechno : TechnoVectorAggressive)
				EventExt::TogglePassiveAcquireMode::Raise(pTechno, PassiveAcquireMode::Normal);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:AGGRESSIVE_MODE_OFF", L"%i unit(s) ceased Aggressive Mode."), TechnoVectorAggressive.size());
			MessageListClass::Instance->PrintMessage(buffer);
		}
		else
		{
			for (const auto& pTechno : TechnoVectorNonAggressive)
				EventExt::TogglePassiveAcquireMode::Raise(pTechno, PassiveAcquireMode::Aggressive);

			wchar_t buffer[0x100];
			swprintf_s(buffer, GeneralUtils::LoadStringUnlessMissing("MSG:AGGRESSIVE_MODE_ON", L"%i unit(s) entered Aggressive Mode."), TechnoVectorNonAggressive.size());
			MessageListClass::Instance->PrintMessage(buffer);
		}
	}
}