#include "GenericPrerequisite.h"

#include <Utilities/TemplateDef.h>

#include <CCINIClass.h>

#include <Ext/TechnoType/Body.h>

Enumerable<GenericPrerequisite>::container_t Enumerable<GenericPrerequisite>::Array;

const char* Enumerable<GenericPrerequisite>::GetMainSection()
{
	return "GenericPrerequisites";
}

GenericPrerequisite::GenericPrerequisite(const char* const pTitle)
	: Enumerable<GenericPrerequisite>(pTitle)
{ }

void GenericPrerequisite::Parse(CCINIClass* pINI, const char* section, const char* key, ValueableVector<int>& Vec)
{
	if (pINI->ReadString(section, key, Phobos::readDefval, Phobos::readBuffer) > 0)
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
				else if (!GameStrings::IsBlank(cur)) {
					Debug::INIParseFailed(section, key, cur , "Expect valid GenericPrerequisite data");
				}
			}
		}
	}
}

void GenericPrerequisite::Parse(CCINIClass* pINI, const char* section, const char* key, DynamicVectorClass<int>& Vec)
{
	if (pINI->ReadString(section, key, Phobos::readDefval, Phobos::readBuffer) > 0)
	{
		Vec.reset();

		char* context = nullptr;
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = BuildingTypeClass::FindIndexById(cur);
			if (idx > -1)
			{
				Vec.push_back(idx);
			}
			else
			{
				idx = GenericPrerequisite::FindIndexById(cur);
				if (idx > -1)
				{
					Vec.push_back(-1 - idx);
				}
				else
				{
					Debug::INIParseFailed(section, key, cur, "Expect valid GenericPrerequisite data");
				}
			}
		}
	}
}

void GenericPrerequisite::LoadFromINI(CCINIClass* pINI)
{
	const char* section = GenericPrerequisite::GetMainSection();
	INI_EX iniEx { pINI };

	//char generalbuf[0x80];

	char name[0x80];
	strcpy_s(name, this->Name.data());

	_strlwr_s(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	std::string _buffer("Prerequisite");
	_buffer += name;
	GenericPrerequisite::Parse(pINI, GameStrings::General(), _buffer.c_str(), this->Prereqs);
	GenericPrerequisite::Parse(pINI, section, this->Name.data(), this->Prereqs);
	_buffer += "Alternate";
	this->Alternates.Read(iniEx, GameStrings::General(), _buffer.c_str());
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

void GenericPrerequisite::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (!pINI)
		return;

	for(auto& defaultItem : Array) { //load all the default data first
		defaultItem->LoadFromINI(pINI);
	}

	const char* pSection = GetMainSection();

	if (!pINI->GetSection(pSection))
		return;

	auto const pkeyCount = pINI->GetKeyCount(pSection);

	if (!pkeyCount)
		return;

	if (pkeyCount > (int)Array.size())
		Array.reserve(pkeyCount);

	for (int i = 0; i < pkeyCount; ++i) { //load for all keys

		FindOrAllocate(pINI->GetKeyName(pSection, i))->LoadFromINI(pINI);
	}
}

bool Prereqs::HouseOwnsGeneric(HouseClass const* const pHouse, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	const auto idxPrereq = static_cast<size_t>(-1 - Index);

	if (idxPrereq < GenericPrerequisite::Array.size()) {
		for (const auto& index : GenericPrerequisite::Array[idxPrereq]->Prereqs) {
			if (Prereqs::HouseOwnsSpecific(pHouse, index)) {
				return true;
			}
		}

		for (const auto& pType : GenericPrerequisite::Array[idxPrereq]->Alternates) {
			if (pHouse->CountOwnedNow(pType)) {
				return true;
			}
		}

	}

	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass const* const pHouse, int const Index)
{
	const auto pType = BuildingTypeClass::Array->Items[Index];
	const auto pPowerup = pType->PowersUpBuilding;

	if (*pPowerup)
	{
		auto const pCore = BuildingTypeClass::Find(pPowerup);

		if (!pCore || pHouse->ActiveBuildingTypes.get_count(pCore->ArrayIndex) < 1) {
			return false;
		}

		for (auto const& pBld : pHouse->Buildings)
		{
			const auto Types = pBld->GetTypes();

			if (Types[0] != pCore) {
				continue;
			}

			if(Types[1] == pType || Types[2] == pType || Types[3] == pType) {
				return true;
			}
		}

		return false;
	}
	else
	{
		const int count = pHouse->ActiveBuildingTypes.get_count(Index);
		//Debug::LogInfo(__FUNCTION__" [0x%x - %s]Trying to find [(%d)%s] count %d" , pHouse , pHouse->get_ID(), Index , pType->ID , count);
		return  count > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass const* const pHouse, int const Index)
{
	return Index < 0
		? Prereqs::HouseOwnsGeneric(pHouse, Index)
		: Prereqs::HouseOwnsSpecific(pHouse, Index)
		;
}

bool Prereqs::HouseOwnsAll(HouseClass const* const pHouse, const Iterator<int> list)
{
	for (const auto index : list) {
		if (!Prereqs::HouseOwnsPrereq(pHouse, index)) {
			return false;
		}
	}

	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass const* const pHouse, const Iterator<int> list)
{
	for (const auto index : list) {
		if (Prereqs::HouseOwnsPrereq(pHouse, index)) {
			return true;
		}
	}

	return false;
}

bool Prereqs::ListContainsSpecific(Iterator<BuildingTypeClass*> items, int const Index) {
	const auto lookingfor = BuildingTypeClass::Array->Items[Index];

	return std::ranges::any_of(items, [&](BuildingTypeClass* item) {
		return item == lookingfor;
	});
}

bool Prereqs::ListContainsGeneric(Iterator<BuildingTypeClass*> items, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	const auto idxPrereq = static_cast<size_t>(-1 - Index);
	if (idxPrereq < GenericPrerequisite::Array.size()) {
		for (const auto& index : GenericPrerequisite::Array[idxPrereq]->Prereqs) {
			if (Prereqs::ListContainsSpecific(items, index)) {
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::ListContainsPrereq(Iterator<BuildingTypeClass*> items, int Index)
{
	return Index < 0
		? Prereqs::ListContainsGeneric(items, Index)
		: Prereqs::ListContainsSpecific(items, Index)
		;
}

bool Prereqs::ListContainsAll(Iterator<BuildingTypeClass*> items, const Iterator<int> intit)
{
	for (auto init : intit) {
		if (!Prereqs::ListContainsPrereq(items, init)) {
			return false;
		}
	}

	return true;
}

bool Prereqs::ListContainsAny(Iterator<BuildingTypeClass*> items, const Iterator<int> Requirements)
{
	for (const auto index : Requirements) {
		if (Prereqs::ListContainsPrereq(items, index)) {
			return true;
		}
	}

	return false;
}

bool Prereqs::PrerequisitesListed(Iterator<BuildingTypeClass*> items, TechnoTypeClass* pItem)
{
	for(auto& prereq : TechnoTypeExtContainer::Instance.Find(pItem)->Prerequisites) {
		if (Prereqs::ListContainsAll(items , prereq)) {
			return true;
		}
	}

	return false;
}
