#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Constructs.h>
#include <Utilities/VersesData.h>

class ArmorTypeClass final : public Enumerable<ArmorTypeClass>
{
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
	virtual ~ArmorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static bool IsDefault(const char* pName);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);
	static void LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);
	static void LoadForWarhead_NoParse(WarheadTypeClass* pWH);
	static void PrepareForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);

	static void EvaluateDefault();

private:
	template <typename T>
	void Serialize(T& Stm);
};