#include "PoweredUnitClass.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Misc/Ares/Hooks/Header.h>

#include <InfantryClass.h>

bool PoweredUnitClass::IsPoweredBy(HouseClass* const pOwner) const
{
	auto const pType = this->Techno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto const& PoweredBy = pTypeExt->PoweredBy;

	for (auto const& pBuilding : pOwner->Buildings)
	{
		auto const inArray = PoweredBy.Contains(pBuilding->Type);

		if (inArray && !pBuilding->BeingWarpedOut && !pBuilding->IsUnderEMP())
		{
			if (TechnoExt_ExtData::IsOperated(pBuilding) && pBuilding->IsPowerOnline())
			{
				return true;
			}
		}
	}

	return false;
}

void PoweredUnitClass::PowerUp()
{
	auto const pTechno = this->Techno;
	if (!pTechno->IsUnderEMP() && TechnoExt_ExtData::IsOperated(pTechno))
	{
		AresEMPulse::DisableEMPEffect2(pTechno);
	}
}

bool PoweredUnitClass::PowerDown()
{
	auto const pTechno = this->Techno;

	if (AresEMPulse::IsDeactivationAdvisableB(pTechno))
	{
		// destroy if EMP.Threshold would crash this unit when in air
		if (AresEMPulse::EnableEMPEffect2(pTechno)
			|| (TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType())->EMP_Threshold
				&& pTechno->IsInAir()))
		{
			return false;
		}
	}

	return true;
}

bool PoweredUnitClass::Update()
{
	if ((Unsorted::CurrentFrame - this->LastScan) < ScanInterval)
	{
		return true;
	}

	auto const pTechno = this->Techno;

	if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
	{
		return true;
	}

	const auto curMission = pTechno->CurrentMission;
	this->LastScan = Unsorted::CurrentFrame;

	if (curMission == Mission::Selling || curMission == Mission::Construction)
		return true;

	const auto queueMission = pTechno->QueuedMission;

	if (queueMission == Mission::Selling || queueMission == Mission::Construction)
		return true;


	auto const pOwner = pTechno->Owner;
	auto const hasPower = this->IsPoweredBy(pOwner);

	this->Powered = hasPower;

	if (hasPower && pTechno->Deactivated)
	{
		this->PowerUp();
	}
	else if (!hasPower && !pTechno->Deactivated)
	{
		// don't shutdown units inside buildings (warfac, barracks, shipyard) because that locks up the factory and the robot tank did it
		auto const whatAmI = pTechno->WhatAmI();
		if ((whatAmI != InfantryClass::AbsID && whatAmI != UnitClass::AbsID) || (!pTechno->GetCell()->GetBuilding()))
		{
			return this->PowerDown();
		}
	}

	return true;
}

