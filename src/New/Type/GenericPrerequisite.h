#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Iterator.h>
#include <Utilities/TemplateDefB.h>

class GenericPrerequisite final : public Enumerable<GenericPrerequisite>
{
public:
	GenericPrerequisite(const char* pTitle);

	virtual ~GenericPrerequisite() override;

	virtual void LoadFromINI(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;

	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	static void AddDefaults();
	static void Parse(CCINIClass* pINI, const char* section, const char* key, ValueableVector<int>& Vec);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	ValueableVector<int> Prereqs;
	ValueableVector<TechnoTypeClass*> Alternates;
};

class Prereqs
{
public:
	using BTypeIter = Iterator<BuildingTypeClass*>;

	static bool HouseOwnsGeneric(HouseClass const* pHouse, int Index);
	static bool HouseOwnsSpecific(HouseClass const* pHouse, int Index);
	static bool HouseOwnsPrereq(HouseClass const* pHouse, int Index);

	static bool HouseOwnsAll(HouseClass const* pHouse, const DynamicVectorClass<int>& list);
	static bool HouseOwnsAny(HouseClass const* pHouse, const DynamicVectorClass<int>& list);

	static bool ListContainsGeneric(const BTypeIter& List, int Index);
	static bool ListContainsSpecific(const BTypeIter& List, int Index);
	static bool ListContainsPrereq(const BTypeIter& List, int Index);

	static bool ListContainsAll(const BTypeIter& List, const DynamicVectorClass<int>& Requirements);
	static bool ListContainsAny(const BTypeIter& List, const DynamicVectorClass<int>& Requirements);
};