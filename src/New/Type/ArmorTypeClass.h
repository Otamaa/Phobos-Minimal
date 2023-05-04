#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Constructs.h>
#include <Utilities/VersesData.h>

class ArmorTypeClass final : public Enumerable<ArmorTypeClass>
{
public:

	int DefaultTo;
	FixedString<0x40> DefaultString;
	VersesData DefaultVersesValue;

	FixedString<0x50> BaseTag;
	FixedString<0x50> FF_Tag;
	FixedString<0x50> RT_Tag;
	FixedString<0x50> PA_Tag;
	FixedString<0x50> HitAnim_Tag;

	ArmorTypeClass(const char* const pTitle);
	virtual ~ArmorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static bool IsDefault(const char* pName);
	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);
	static void LoadForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);
	static void PrepareForWarhead(CCINIClass* pINI, WarheadTypeClass* pWH);

	void EvaluateDefault();

private:
	template <typename T>
	void Serialize(T& Stm);
};