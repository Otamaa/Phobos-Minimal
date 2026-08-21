#include "Body.h"

#include <Ext/Cell/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>

bool SmudgeExtData::ShouldRemoveSmudgeCell(const int index, const int time, const int current)
{
	const auto cell = CellStruct { static_cast<short>(index & 511), static_cast<short>(index >> 9) };

	if (const auto pCell = MapClass::Instance->TryGetCellAt(cell))
	{
		if (pCell->SmudgeTypeIndex != -1)
		{
			const auto pCellExt = CellExtContainer::Instance.Find(pCell);

			if ((pCellExt->SmudgeGenerate + time) > current)
				return false;

			const auto state = pCellExt->SmudgeState;

			if (state != BlitterFlags::TransLucent75)
			{
				pCellExt->SmudgeGenerate = current;
				pCellExt->SmudgeState = static_cast<BlitterFlags>(static_cast<size_t>(state) + 2u);
				pCell->MarkForRedraw();
				return false;
			}

			pCell->SmudgeTypeIndex = -1;
			pCell->MarkForRedraw();
		}
	}

	return true;
}

void SmudgeExtData::UpdateSmudgeState() {
	const int time = FakeRulesClass::Instance->SmudgeUpdateTime;

	if (time > 0)
	{
		auto& s = ScenarioExtData::Instance()->Smudges;

		if (!s.empty())
		{
			const int current = Unsorted::CurrentFrame();

			for (auto it = s.begin(); it != s.end(); )
			{
				if (ShouldRemoveSmudgeCell(*it, time, current))
					it = s.erase(it);
				else
					++it;
			}
		}
	}
}