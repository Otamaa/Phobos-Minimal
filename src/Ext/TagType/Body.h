#pragma once
#include <TagTypeClass.h>

class NOVTABLE FakeTagTypeClass : public TagTypeClass
{
public:
	static bool __fastcall _LoadEntryINI(CCINIClass* pINI);
	bool _LoadFromINI(CCINIClass* pINI);
	bool _WriteToINI(CCINIClass* pINI);

}; static_assert(sizeof(FakeTagTypeClass) == sizeof(TagTypeClass), "Invalid Size !");