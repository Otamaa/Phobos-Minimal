#include "GenericPrerequisite.h"

#include <Utilities/TemplateDef.h>

#include <CCINIClass.h>

Enumerable<GenericPrerequisite>::container_t Enumerable<GenericPrerequisite>::Array;

const char* Enumerable<GenericPrerequisite>::GetMainSection()
{
	return "GenericPrerequisites";
}

GenericPrerequisite::GenericPrerequisite(const char* const pTitle)
	: Enumerable<GenericPrerequisite>(pTitle)
{ }

GenericPrerequisite::~GenericPrerequisite() = default;

void GenericPrerequisite::Parse(CCINIClass* pINI, const char* section, const char* key, ValueableVector<int>& Vec)
{
	if (pINI->ReadString(section, key, Phobos::readDefval, Phobos::readBuffer))
	{
		Vec.clear();

		char* context = nullptr;
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); 
			cur; 
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = BuildingTypeClass::FindIndexById(cur);
			if (idx > -1) {
				Vec.push_back(idx);
			} else {
				idx = GenericPrerequisite::FindIndexById(cur);
				if (idx > -1) {
					Vec.push_back(-1 - idx);
				}
				else {
					Debug::INIParseFailed(section, key, cur , "Expect valid GenericPrerequisite data");
				}
			}
		}
	}
}

void GenericPrerequisite::LoadFromINI(CCINIClass* pINI)
{
	const char* section = GenericPrerequisite::GetMainSection();
	INI_EX iniEx { pINI };

	char generalbuf[0x80];

	char name[0x80];
	strcpy_s(name, this->Name.data());

	_strlwr_s(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	IMPL_SNPRNINTF(generalbuf, sizeof(generalbuf), "Prerequisite%s", name);
	GenericPrerequisite::Parse(pINI, GameStrings::General(), generalbuf, this->Prereqs);

	GenericPrerequisite::Parse(pINI, section, this->Name, this->Prereqs);

	IMPL_SNPRNINTF(generalbuf, sizeof(generalbuf), "Prerequisite%sAlternate", name);
	this->Alternates.Read(iniEx, GameStrings::General(), generalbuf);
}

void GenericPrerequisite::LoadFromStream(PhobosStreamReader& Stm)
{
	Stm
		.Process(this->Prereqs)
		.Process(this->Alternates);
}

void GenericPrerequisite::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm
		.Process(this->Prereqs)
		.Process(this->Alternates);
}

void GenericPrerequisite::AddDefaults()
{
	FindOrAllocate("POWER");
	FindOrAllocate("FACTORY");
	FindOrAllocate("BARRACKS");
	FindOrAllocate("RADAR");
	FindOrAllocate("TECH");
	FindOrAllocate("PROC");
}

void GenericPrerequisite::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (!pINI)
		return;

	const char* pSection = GetMainSection();

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);

	if (!pkeyCount)
		return;

	if (pkeyCount > (int)Array.size())
		Array.reserve(pkeyCount);

	for (int i = 0; i < pkeyCount; ++i)
	{
		const auto pKeyHere = pINI->GetKeyName(pSection, i);
		if (auto const pAlloc = Find(pKeyHere))
			pAlloc->LoadFromINI(pINI);
		else
			Allocate(pKeyHere)->LoadFromINI(pINI);
	}
}

bool Prereqs::HouseOwnsGeneric(HouseClass const* const pHouse, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	auto const idxPrereq = static_cast<unsigned int>(-1 - Index);
	if (idxPrereq < GenericPrerequisite::Array.size())
	{
		auto const& Prereq = GenericPrerequisite::Array[idxPrereq];
		for (const auto& index : Prereq->Prereqs)
		{
			if (Prereqs::HouseOwnsSpecific(pHouse, index))
			{
				return true;
			}
		}
		for (const auto& pType : Prereq->Alternates)
		{
			if (pHouse->CountOwnedNow(pType))
			{
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass const* const pHouse, int const Index)
{
	auto const pType = BuildingTypeClass::Array->Items[Index];
	auto const pPowerup = pType->PowersUpBuilding;

	if (*pPowerup)
	{
		auto const pCore = BuildingTypeClass::Find(pPowerup);
		if (!pCore || pHouse->ActiveBuildingTypes.GetItemCount(pCore->ArrayIndex) < 1)
		{
			return false;
		}
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type != pCore)
			{
				continue;
			}
			for (const auto& pUpgrade : pBld->Upgrades)
			{
				if (pUpgrade == pType)
				{
					return true;
				}
			}
		}
		return false;
	}
	else
	{
		return pHouse->ActiveBuildingTypes.GetItemCount(Index) > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass const* const pHouse, int const Index)
{
	return Index < 0
		? Prereqs::HouseOwnsGeneric(pHouse, Index)
		: Prereqs::HouseOwnsSpecific(pHouse, Index)
		;
}

bool Prereqs::HouseOwnsAll(HouseClass const* const pHouse, const DynamicVectorClass<int>& list)
{
	for (const auto& index : list)
	{
		if (!Prereqs::HouseOwnsPrereq(pHouse, index))
		{
			return false;
		}
	}
	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass const* const pHouse, const DynamicVectorClass<int>& list)
{
	for (const auto& index : list)
	{
		if (Prereqs::HouseOwnsPrereq(pHouse, index))
		{
			return true;
		}
	}
	return false;
}

bool Prereqs::ListContainsSpecific(const BTypeIter& List, int const Index)
{
	auto const pItem = BuildingTypeClass::Array->Items[Index];
	return List.contains(pItem);
}

bool Prereqs::ListContainsGeneric(const BTypeIter& List, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	auto const idxPrereq = static_cast<unsigned int>(-1 - Index);
	if (idxPrereq < GenericPrerequisite::Array.size())
	{
		const auto& dvc = GenericPrerequisite::Array[idxPrereq]->Prereqs;
		for (const auto& index : dvc)
		{
			if (Prereqs::ListContainsSpecific(List, index))
			{
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::ListContainsPrereq(const BTypeIter& List, int Index)
{
	return Index < 0
		? Prereqs::ListContainsGeneric(List, Index)
		: Prereqs::ListContainsSpecific(List, Index)
		;
}

bool Prereqs::ListContainsAll(const BTypeIter& List, const DynamicVectorClass<int>& Requirements)
{
	for (const auto& index : Requirements)
	{
		if (!Prereqs::ListContainsPrereq(List, index))
		{
			return false;
		}
	}
	return true;
}

bool Prereqs::ListContainsAny(const BTypeIter& List, const DynamicVectorClass<int>& Requirements)
{
	for (const auto& index : Requirements)
	{
		if (Prereqs::ListContainsPrereq(List, index))
		{
			return true;
		}
	}
	return false;
}
