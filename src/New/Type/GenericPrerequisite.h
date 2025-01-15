#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Iterator.h>
#include <Utilities/TemplateDefB.h>

class GenericPrerequisite final : public Enumerable<GenericPrerequisite>
{
public:
	GenericPrerequisite(const char* pTitle);

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		Array.reserve(6u);

		FindOrAllocate(GameStrings::POWER());
		FindOrAllocate(GameStrings::FACTORY());
		FindOrAllocate(GameStrings::BARRACKS());
		FindOrAllocate(GameStrings::RADAR());
		FindOrAllocate("TECH");
		FindOrAllocate("PROC");
	}

	static void Parse(CCINIClass* pINI, const char* section, const char* key, ValueableVector<int>& Vec);
	static void Parse(CCINIClass* pINI, const char* section, const char* key, DynamicVectorClass<int>& Vec);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	ValueableVector<int> Prereqs;
	ValueableVector<TechnoTypeClass*> Alternates;
};

class Prereqs
{
public:

	static bool HouseOwnsGeneric(HouseClass const* pHouse, int Index);
	static bool HouseOwnsSpecific(HouseClass const* pHouse, int Index);
	static bool HouseOwnsPrereq(HouseClass const* pHouse, int Index);

	static bool HouseOwnsAll(HouseClass const* pHouse, const DynamicVectorClass<int>& list);
	static bool HouseOwnsAll(HouseClass const* pHouse, int* intitems, int intsize);
	static bool HouseOwnsAny(HouseClass const* pHouse, const DynamicVectorClass<int>& list);
	static bool HouseOwnsAny(HouseClass const* pHouse, int* intitems, int intsize);

	static bool ListContainsGeneric(BuildingTypeClass** items, int size, int Index);
	static bool ListContainsSpecific(BuildingTypeClass** items, int size, int Index);
	static bool ListContainsPrereq(BuildingTypeClass** items, int size, int Index);

	static bool ListContainsAll(BuildingTypeClass** items, int size, int* intitems, int intsize);
	static bool ListContainsAny(BuildingTypeClass** items, int size, const DynamicVectorClass<int>& Requirements);

	static bool PrerequisitesListed(BuildingTypeClass** items , int size , TechnoTypeClass* pItem);
};