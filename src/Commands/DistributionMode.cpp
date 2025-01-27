#include "DistributionMode.h"

#include <Ext/TechnoType/Body.h>
#include <Utilities/Helpers.h>
#include <Helpers/Macro.h>

#include <HouseClass.h>
#include <PlanningTokenClass.h>

int DistributionMode1CommandClass::Mode = 0;
int DistributionMode2CommandClass::Mode = 0;

const char* DistributionMode1CommandClass::GetName() const
{
	return "Distribution Mode Spread";
}

const wchar_t* DistributionMode1CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD", L"Distribution spread");
}

const wchar_t* DistributionMode1CommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionMode1CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_SPREAD_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the search range");
}

void DistributionMode1CommandClass::Execute(WWKey eInput) const
{
	DistributionMode1CommandClass::Mode = ((DistributionMode1CommandClass::Mode + 1) & 3);
}

const char* DistributionMode2CommandClass::GetName() const
{
	return "Distribution Mode Filter";
}

const wchar_t* DistributionMode2CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER", L"Distribution filter");
}

const wchar_t* DistributionMode2CommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* DistributionMode2CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISTR_FILTER_DESC", L"Automatically and averagely select similar targets around the original target. This is for changing the filter criteria");
}

void DistributionMode2CommandClass::Execute(WWKey eInput) const
{
	DistributionMode2CommandClass::Mode = ((DistributionMode2CommandClass::Mode + 1) & 3);
}

void DistributionMode::Draw(ObjectClass* const pTarget , const Action mouseAction)
{
	const auto count = ObjectClass::CurrentObjects->Count;

	if (count > 0)
	{
		const auto mode1 = DistributionMode1CommandClass::Mode;
		const auto mode2 = DistributionMode2CommandClass::Mode;

		// Distribution mode main
		if (mode1 && count > 1 && mouseAction != Action::NoMove && !PlanningNodeClass::PlanningModeActive && (pTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && !pTarget->IsInAir())
		{
			const auto pSpecial = HouseClass::FindSpecial();
			const auto pCivilian = HouseClass::FindCivilianSide();
			const auto pNeutral = HouseClass::FindNeutral();

			const auto pTargetHouse = static_cast<TechnoClass*>(pTarget)->Owner;
			const bool targetIsNeutral = pTargetHouse == pSpecial || pTargetHouse == pCivilian || pTargetHouse == pNeutral;

			const auto range = (2 << mode1);
			const auto pItems = Helpers::Alex::getCellSpreadItems(pTarget->Location, range);
			std::map<TechnoClass*, int> record;
			int current = 1;

			for (const auto& pItem : pItems)
				record[pItem] = 0;

			for (const auto& pSelect : ObjectClass::CurrentObjects())
			{
				TechnoClass* pCanTarget = nullptr;
				TechnoClass* pNewTarget = nullptr;

				for (const auto& [pItem, num] : record)
				{
					if (pSelect->MouseOverObject(pItem) == mouseAction && (targetIsNeutral || (pItem->Owner != pSpecial && pItem->Owner != pCivilian && pItem->Owner != pNeutral))
						&& (mode2 < 2 || (pItem->WhatAmI() == pTarget->WhatAmI()
							&& (mode2 < 3 || TechnoTypeExtData::GetSelectionGroupID(pItem->GetTechnoType()) == TechnoTypeExtData::GetSelectionGroupID(pTarget->GetTechnoType())))))
					{
						pCanTarget = pItem;

						if (num < current)
						{
							pNewTarget = pCanTarget;
							break;
						}
					}
				}

				if (!pNewTarget)
				{
					if (pCanTarget)
					{
						++current;
						pNewTarget = pCanTarget;
					}
				}

				if (pNewTarget)
				{
					if (record.contains(pNewTarget))
						++record[pNewTarget];

					pSelect->ObjectClickedAction(mouseAction, pNewTarget, false);
				}
				else
				{
					const auto currentAction = pSelect->MouseOverObject(pTarget);

					if (mode2 && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
						static_cast<TechnoClass*>(pSelect)->ClickedMission(Mission::Area_Guard, reinterpret_cast<ObjectClass*>(pSelect->GetCellAgain()), nullptr, nullptr);
					else
						pSelect->ObjectClickedAction(currentAction, pTarget, false);
				}

				Unsorted::MoveFeedback = false;
			}
		}
		else // Vanilla
		{
			for (const auto& pSelect : ObjectClass::CurrentObjects())
			{
				const auto currentAction = pSelect->MouseOverObject(pTarget);

				if (mode2 && mouseAction != Action::NoMove && currentAction == Action::NoMove && (pSelect->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
					static_cast<TechnoClass*>(pSelect)->ClickedMission(Mission::Area_Guard, reinterpret_cast<ObjectClass*>(pSelect->GetCellAgain()), nullptr, nullptr);
				else
					pSelect->ObjectClickedAction(currentAction, pTarget, false);

				Unsorted::MoveFeedback = false;
			}
		}
	}
}

void DistributionMode::DrawRadialIndicator()
{
	const auto mode1 = DistributionMode1CommandClass::Mode;
	const auto mode2 = DistributionMode2CommandClass::Mode;

	if (mode1 || mode2)
	{
		const auto pCell = MapClass::Instance->GetCellAt(DisplayClass::Instance->CurrentFoundation_CenterCell);
		const auto color = ((mode2 > 1)
			? ((mode2 == 3) ? ColorStruct { 255, 0, 0 } : ColorStruct { 200, 200, 0 })
			: (mode2 == 1) ? ColorStruct { 0, 100, 255 } : ColorStruct { 0, 255, 50 });

		Draw_Radial_Indicator(false, true, pCell->GetCoords(), color, static_cast<float>(mode1 ? (2 << mode1) : 0.5), false, true);
	}
}

DEFINE_HOOK(0x4AE818, DisplayClass_sub_4AE750_AutoDistribution, 0xA)
{

	GET(ObjectClass* const, pTarget, EBP);
	GET_STACK(const Action, mouseAction, STACK_OFFSET(0x20, 0xC));
	DistributionMode::Draw(pTarget, mouseAction);
	return 0x4AE85C;
}

DEFINE_HOOK(0x6DBE74, TacticalClass_DrawAllRadialIndicators_DrawDistributionRange, 0x5)
{
	DistributionMode::DrawRadialIndicator();
	return 0;
}