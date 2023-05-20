#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class ImmunityTypeClass final : public Enumerable<ImmunityTypeClass>
{
public:

	ImmunityTypeClass(const char* const pTitle) : Enumerable<ImmunityTypeClass>(pTitle)
	{ }

	virtual ~ImmunityTypeClass() override = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override { }
	virtual void SaveToStream(PhobosStreamWriter& Stm) override { }
	static void LoadFromINIList(CCINIClass* pINI, bool bDebug = false);
};