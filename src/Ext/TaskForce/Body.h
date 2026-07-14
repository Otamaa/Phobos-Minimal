#pragma once
#include <TaskForceClass.h>

class NOVTABLE FakTaskForceClass : public TaskForceClass
{
public:
	static bool __fastcall _LoadEntryINI(CCINIClass* pINI, TaskForceType type);
	bool _LoadFromINI(CCINIClass* pINI);
	bool _WriteToINI(CCINIClass* pINI);

}; static_assert(sizeof(FakTaskForceClass) == sizeof(TaskForceClass), "Invalid Size !");