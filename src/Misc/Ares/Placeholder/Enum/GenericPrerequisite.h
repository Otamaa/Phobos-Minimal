#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Iterator.h>

class GenericPrerequisite final : public Enumerable<GenericPrerequisite>
{
public:
	GenericPrerequisite(const char* pTitle);

	virtual ~GenericPrerequisite() override;

	virtual void LoadFromINI(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;

	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	static void AddDefaults();

	std::vector<int> Prereqs;
	std::vector<TechnoTypeClass*> Alternates;
};

class Prereqs
{
public:
	using BTypeIter = Iterator<BuildingTypeClass*>;
	using IntIter = Iterator<int>;

	static void Parse(CCINIClass* pINI, const char* section, const char* key, std::vector<int>& Vec);
	static void ParseAlternate(CCINIClass* pINI, const char* section, const char* key, std::vector<TechnoTypeClass*>& Vec);

	static bool HouseOwnsGeneric(HouseClass const* pHouse, int Index);
	static bool HouseOwnsSpecific(HouseClass const* pHouse, int Index);
	static bool HouseOwnsPrereq(HouseClass const* pHouse, int Index);

	static bool HouseOwnsAll(HouseClass const* pHouse, const IntIter& list);
	static bool HouseOwnsAny(HouseClass const* pHouse, const IntIter& list);

	static bool ListContainsGeneric(const BTypeIter& List, int Index);
	static bool ListContainsSpecific(const BTypeIter& List, int Index);
	static bool ListContainsPrereq(const BTypeIter& List, int Index);

	static bool ListContainsAll(const BTypeIter& List, const IntIter& Requirements);
	static bool ListContainsAny(const BTypeIter& List, const IntIter& Requirements);
};