#include "TechTreeTypeClass.h"

#include <HouseClass.h>

#include <Utilities/TemplateDef.h>
#include "Utilities/Debug.h"

template<>
const char* Enumerable<TechTreeTypeClass>::GetMainSection()
{
	return "TechTreeTypes";
}

void TechTreeTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name.c_str();

	INI_EX exINI(pINI);

	this->SideIndex.Read(exINI, section, "SideIndex");
	this->ConstructionYard.Read(exINI, section, "ConstructionYard");
	this->BuildPower.Read(exINI, section, "BuildPower");
	this->BuildRefinery.Read(exINI, section, "BuildRefinery");
	this->BuildBarracks.Read(exINI, section, "BuildBarracks");
	this->BuildWeapons.Read(exINI, section, "BuildWeapons");
	this->BuildRadar.Read(exINI, section, "BuildRadar");
	this->BuildHelipad.Read(exINI, section, "BuildHelipad");
	this->BuildNavalYard.Read(exINI, section, "BuildNavalYard");
	this->BuildTech.Read(exINI, section, "BuildTech");
	this->BuildAdvancedPower.Read(exINI, section, "BuildAdvancedPower");
	this->BuildDefense.Read(exINI, section, "BuildDefense");
	this->BuildOther.Read(exINI, section, "BuildOther");
	this->BuildOtherCounts.Read(exINI, section, "BuildOtherCounts");

	for (size_t i = 0; i < BuildOther.size(); i++)
	{
		if (i < BuildOtherCounts.size())
		{
			BuildOtherCountMap[BuildOther[i]] = BuildOtherCounts[i];
		}
		else
		{
			Debug::LogInfo("TechTreeTypeClass::LoadFromINI: BuildOtherCounts is missing count for {}, setting to 0.", BuildOther[i]->Name);
			BuildOtherCountMap[BuildOther[i]] = 0;
		}
	}
}

template <typename T>
void TechTreeTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(SideIndex)
		.Process(ConstructionYard)
		.Process(BuildPower)
		.Process(BuildRefinery)
		.Process(BuildBarracks)
		.Process(BuildRadar)
		.Process(BuildHelipad)
		.Process(BuildNavalYard)
		.Process(BuildTech)
		.Process(BuildAdvancedPower)
		.Process(BuildDefense)
		.Process(BuildOther)
		.Process(BuildOtherCounts)
		.Process(BuildOtherCountMap)
		;
}

void TechTreeTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void TechTreeTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
