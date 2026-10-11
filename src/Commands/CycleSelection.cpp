#include "CycleSelection.h"

#include <Utilities/GeneralUtils.h>
#include <Misc/CSF.h>

namespace CycleSelection
{
	constexpr int NavCycleMode_CycleSelection = 6;

	// Not persisted in savegames.
	std::vector<ObjectClass*> Objects;
	int Index = -1;

	void Reset()
	{
		Objects.clear();
		Index = -1;
	}

	// The object the cycle currently sits on if it is still usable, null otherwise.
	ObjectClass* GetCyclable(int index)
	{
		if (index < 0 || index >= static_cast<int>(Objects.size()))
			return nullptr;

		const auto pObject = flag_cast_to<ObjectClass*>(Objects[index]);

		if (!pObject || pObject->Health <= 0 || !pObject->IsAlive || pObject->InLimbo)
			return nullptr;

		const auto pOwner = pObject->GetOwningHouse();

		if (!pOwner || !pOwner->ControlledByCurrentPlayer())
			return nullptr;

		return pObject;
	}
}

const char* CycleSelectionCommandClass::GetName() const
{
	return "Cycle Selection";
}

const wchar_t* CycleSelectionCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_SELECTION", L"Cycle Selection");
}

const wchar_t* CycleSelectionCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* CycleSelectionCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CYCLE_SELECTION_DESC", L"Cycle through the current selection, selecting one object at a time.");
}

void CycleSelectionCommandClass::Execute(WWKey eInput) const
{
	// Same guard vanilla's navigation commands use
	if (Unsorted::MuteSWLaunches())
		return;

	if (Unsorted::NavCycleMode() != CycleSelection::NavCycleMode_CycleSelection)
	{
		CycleSelection::Objects.clear();

		for (const auto pObject : ObjectClass::CurrentObjects())
		{
			CycleSelection::Objects.push_back(pObject);
		}

		CycleSelection::Index = -1;
	}

	const int count = static_cast<int>(CycleSelection::Objects.size());

	if (count <= 0)
	{
		MessageListClass::Instance->PrintMessage(CSFLoader::FetchStringManager("MSG:NothingSelected",nullptr, nullptr, -1),
			RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		return;
	}

	ObjectClass* pTarget = nullptr;
	int targetIndex = -1;

	// Step onto the next object that is still usable
	while (!CycleSelection::Objects.empty())
	{
		int index = CycleSelection::Index + 1;

		if (index >= static_cast<int>(CycleSelection::Objects.size()))
			index = 0;

		if (const auto pObject = CycleSelection::GetCyclable(index))
		{
			pTarget = pObject;
			targetIndex = index;
			break;


		}

		CycleSelection::Objects.erase(CycleSelection::Objects.begin() + index);

		if (index <= CycleSelection::Index)
			--CycleSelection::Index;
	}

	if (!pTarget)
	{
		CycleSelection::Reset();
		MessageListClass::Instance->PrintMessage(CSFLoader::FetchStringManager(GameStrings::TXT_NOTHING_SELECTED(), nullptr, nullptr, -1),
			RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
		return;
	}

	MapClass::UnselectAll();

	if (pTarget->Select())
	{
		CycleSelection::Index = targetIndex;
		MapClass::Instance->MarkNeedsRedraw(1);
		// UnselectAll and Select sets NavCycleMode to 0
		Unsorted::NavCycleMode = CycleSelection::NavCycleMode_CycleSelection;
	}
}