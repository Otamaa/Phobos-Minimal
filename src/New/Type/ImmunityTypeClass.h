#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class ImmunityTypeClass final : public Enumerable<ImmunityTypeClass>
{
public:

	ImmunityTypeClass(const char* const pTitle) : Enumerable<ImmunityTypeClass>(pTitle)
	{ }

	static void LoadFromINIList(CCINIClass* pINI, bool bDebug = false);

	void LoadFromStream(PhobosStreamReader& Stm) { };
	void SaveToStream(PhobosStreamWriter& Stm) { };
};
