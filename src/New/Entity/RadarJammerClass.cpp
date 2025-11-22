#include "RadarJammerClass.h"

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Misc/Ares/Hooks/Header.h>


//! \param TargetBuilding The building whose eligibility to check.
bool RadarJammerClass::IsEligible(BuildingClass* TargetBuilding)
{
	/* Current requirements for being eligible:
		- not an ally (includes ourselves)
		- either a radar or a spysat
	*/
	auto const pExt = TechnoTypeExtContainer::Instance.Find(this->AttachedToObject->GetTechnoType());
	if (EnumFunctions::CanTargetHouse(pExt->RadarJamHouses, this->AttachedToObject->Owner, TargetBuilding->Owner)) {
		if(pExt->RadarJamIgnore.Contains(TargetBuilding->Type)
				|| (!pExt->RadarJamAffect.empty() && !pExt->RadarJamAffect.Contains(TargetBuilding->Type)))
				return false;

		if (TargetBuilding->Type->Radar || TargetBuilding->Type->SpySat)
			return true;

		for (auto pType : TargetBuilding->GetTypes()) {
			if (pType && pType->SpySat) {

				if(pExt->RadarJamIgnore.Contains(pType)
					|| (!pExt->RadarJamAffect.empty() && !pExt->RadarJamAffect.Contains(pType)))
					return false;

				return true;
			}
		}
	}

	return false;
}

void RadarJammerClass::Update()
{	
	auto const pExt = TechnoTypeExtContainer::Instance.Find(this->AttachedToObject->GetTechnoType());

	// we don't want to scan & crunch numbers every frame - this limits it to ScanInterval frames
	if ((Unsorted::CurrentFrame - this->LastScan) < pExt->RadarJamDelay) {
		return;
	}

	// save the current frame for future reference
	this->LastScan = Unsorted::CurrentFrame;

	// walk through all buildings
	for (auto const curBuilding : *BuildingClass::Array)
	{
		if (!this->IsEligible(curBuilding))
			continue;

		// for each jammable building ...
		// ...check if it's in range, and jam or unjam based on that
		if (this->InRangeOf(curBuilding))
		{
			this->Jam(curBuilding);
		}
		else
		{
			this->Unjam(curBuilding);
		}
	}
}

//! \param TargetBuilding The building to check the distance to.
bool RadarJammerClass::InRangeOf(BuildingClass* TargetBuilding)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(this->AttachedToObject->GetTechnoType());
	auto const& JammerLocation = this->AttachedToObject->Location;
	auto const JamRadiusInLeptons = 256.0 * pExt->RadarJamRadius;

	return TargetBuilding->Location.DistanceFrom(JammerLocation) <= JamRadiusInLeptons;
}

//! \param TargetBuilding The building to jam.
void RadarJammerClass::Jam(BuildingClass* TargetBuilding)
{
	//keep item unique
	auto& jammMap = BuildingExtContainer::Instance.Find(TargetBuilding)->RegisteredJammers;

	jammMap.push_back_unique(this->AttachedToObject);

	if (jammMap.size() >= 1)
	{
		TargetBuilding->Owner->RecheckRadar = true;
	}

	this->Registered = true;
}

//! \param TargetBuilding The building to unjam.
void RadarJammerClass::Unjam(BuildingClass* TargetBuilding) const
{
	//keep item unique
	auto& jammMap = BuildingExtContainer::Instance.Find(TargetBuilding)->RegisteredJammers;
	jammMap.remove(this->AttachedToObject);

	if (jammMap.empty())
	{
		TargetBuilding->Owner->RecheckRadar = true;
	}
}

void RadarJammerClass::UnjamAll()
{
	if (this->Registered)
	{
		this->Registered = false;
		for (auto const item : *BuildingClass::Array)
		{
			this->Unjam(item);
		}
	}
}
