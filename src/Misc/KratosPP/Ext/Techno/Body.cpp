#include "Body.h"

void TechnoExtData::AddGlobalScripts(std::list<std::string>& globalScripts, IExtData* ext)
{
	// Base Component
	globalScripts.push_back(AircraftPut::ScriptName);
	globalScripts.push_back(TechnoStatus::ScriptName);
	globalScripts.push_back(AttachEffect::ScriptName);
}

void TechnoExtData::ClearAllArray(EventSystem* sender, Event e, void* args)
{
	BaseUnitArray.clear();
	BaseStandArray.clear();

	StandArray.clear();
	ImmuneStandArray.clear();

	VirtualUnitArray.clear();
}

TechnoExtDataContainer ExtMap {};
std::map<TechnoClass*, bool> TechnoExtData::BaseUnitArray {};
std::map<TechnoClass*, bool> TechnoExtData::BaseStandArray {};

std::map<TechnoClass*, StandData> TechnoExtData::StandArray {};
std::map<TechnoClass*, StandData> TechnoExtData::ImmuneStandArray {};
std::vector<TechnoClass*> TechnoExtData::VirtualUnitArray {};

HealthTextControlData TechnoExtData::HealthTextControlData {};