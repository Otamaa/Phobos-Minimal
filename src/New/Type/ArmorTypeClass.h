#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/VersesData.h>

class ArmorTypeClass final : public Enumerable<ArmorTypeClass>
{
public:

	int DefaultTo;
	PhobosFixedString<32> DefaultString;
	VersesData DefaultVerses;

	std::string BaseTag;
	std::string FF_Tag;
	std::string RT_Tag;
	std::string PA_Tag;

	ArmorTypeClass(const char* const pTitle) : Enumerable<ArmorTypeClass>(pTitle)
		, DefaultTo{ -1 }
		, DefaultString { }
		, DefaultVerses { }
		, BaseTag {}
		, FF_Tag {}
		, RT_Tag {}
		, PA_Tag {}
	{ 
		char buffer[0x500];
		_snprintf_s(buffer, _TRUNCATE, "%s.%s", "Versus", pTitle);
		BaseTag = buffer;

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.ForceFire", "Versus", pTitle);
		FF_Tag = buffer;

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.Retaliate", "Versus", pTitle);
		RT_Tag = buffer;

		_snprintf_s(buffer, _TRUNCATE, "%s.%s.PassiveAcquire", "Versus", pTitle);
		PA_Tag = buffer;	
	}

	virtual ~ArmorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static bool IsDefault(const char* pName);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);
	static void LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);

	void EvaluateDefault();

private:
	template <typename T>
	void Serialize(T& Stm);
};