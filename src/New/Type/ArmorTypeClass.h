#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class ArmorTypeClass final : public Enumerable<ArmorTypeClass>
{
public:

	int DefaultTo;
	PhobosFixedString<32> DefaultString;

	ArmorTypeClass(const char* const pTitle) : Enumerable<ArmorTypeClass>(pTitle)
		, DefaultTo{ -1 }
		, DefaultString { }
	{ }

	virtual ~ArmorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static bool IsDefault(const char* pName);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	void EvaluateDefault();

private:
	template <typename T>
	void Serialize(T& Stm);
};