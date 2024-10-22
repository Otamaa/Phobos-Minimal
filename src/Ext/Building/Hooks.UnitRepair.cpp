#include "Body.h"

DEFINE_HOOK(0x44BD38, BuildingClass_Mission_Repair_UnitRepair, 0x6)
{
	enum { SkipGameCode = 0x44BD3E };

	GET(FakeBuildingClass*, pThis, EBP);

	double repairRate = pThis->_GetTypeExtData()->Units_RepairRate.Get(RulesClass::Instance->URepairRate);
	__asm { fld repairRate }

	return SkipGameCode;
}

bool SeparateRepair = false;

DEFINE_HOOK(0x44C836, BuildingClass_Mission_Repair_UnitReload, 0x6)
{
	GET(FakeBuildingClass*, pThis, EBP);

	if (pThis->Type->UnitReload)
	{
		auto const pTypeExt = pThis->_GetTypeExtData();

		if (pTypeExt->Units_RepairRate.isset())
		{
			double repairRate = pTypeExt->Units_RepairRate.Get();

			if (repairRate < 0.0)
				return 0;

			int rate = static_cast<int>(std::max(repairRate * 900, 1.0));

			if (!(Unsorted::CurrentFrame % rate))
			{
				SeparateRepair = true;

				for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
				{
					if (auto const pLink = pThis->GetNthLink(i))
					{
						if (!pLink->IsInAir() && pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
							pThis->SendCommand(RadioCommand::RequestRepair, pLink);
					}
				}

				SeparateRepair = false;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F4CF0, TechnoClass_ReceiveCommand_Repair, 0x5)
{
	enum { AnswerNegative = 0x6F4CB4 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pFrom, STACK_OFFSET(0x18, 0x4));

	auto const pType = pThis->GetTechnoType();
	int repairStep = pType->GetRepairStep();
	int repairCost = pType->GetRepairStepCost();

	if (auto const pBuilding = specific_cast<BuildingClass*>(pFrom))
	{
		auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		if (pBuilding->Type->UnitReload && pTypeExt->Units_RepairRate.isset() && !SeparateRepair)
			return AnswerNegative;

		repairStep = pTypeExt->Units_RepairStep.Get(repairStep);
		double repairPercent = pTypeExt->Units_RepairPercent.Get(RulesClass::Instance->RepairPercent);

		if (!pTypeExt->Units_DisableRepairCost) {

			repairCost = static_cast<int>((pType->GetCost() / (pType->Strength / static_cast<double>(repairStep))) * repairPercent);

			if (repairCost < 1)
				repairCost = 1;

		} else {
			repairCost = 0;
		}
	}

	if (repairStep < 1)
		repairStep = 1;

	R->EDI(repairStep);
	R->EBX(repairCost);

	return 0x6F4D26;
}

// Fixes docks not repairing docked aircraft unless they enter the dock first e.g just built ones.
// Also potential edge cases with unusual docking offsets, original had a distance check for 64 leptons which is replaced with IsInAir here.
DEFINE_HOOK(0x44985B, BuildingClass_Mission_Guard_UnitReload, 0x6)
{
	enum { AssignRepairMission = 0x449942 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pLink, EDI);

	if (pThis->Type->UnitReload && pLink->WhatAmI() == AbstractType::Aircraft && !pLink->IsInAir()
		&& pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
	{
		return AssignRepairMission;
	}

	return 0;
}