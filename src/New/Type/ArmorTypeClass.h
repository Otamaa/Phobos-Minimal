#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Constructs.h>

#include <New/Entity/VersesData.h>

class ArmorTypeClass final : public Enumerable<ArmorTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "ArmorTypes";
	static COMPILETIMEEVAL const char* ClassName = "ArmorTypeClass";

	//faster handling for looking up the default chains
	static std::unordered_map<std::string, int> ArmorLookup;

public:

	int DefaultTo;
	std::string DefaultString;
	VersesData DefaultVersesValue;
	bool IsVanillaArmor;

	std::string BaseTag;
	std::string FF_Tag;
	std::string RT_Tag;
	std::string PA_Tag;
	std::string HitAnim_Tag;

	ArmorTypeClass(const char* const pTitle);

	static void AddDefaults();

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void RebuildTags();
	void FreeTags();

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);
	static void LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);
	static void LoadForWarhead_NoParse(WarheadTypeClass* pWH);
	static void PrepareForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);

	static void EvaluateDefault();

	static void Clear()
	{
		Enumerable<ArmorTypeClass>::Clear();
		ArmorLookup.clear();
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};