#pragma once
#include <TaskForceClass.h>

class NOVTABLE FakTaskForceClass : public TaskForceClass
{
public:
	bool _LoadEntryINI(CCINIClass* pINI, bool isGlobal);
	bool _LoadFromINI(CCINIClass* pINI);
	bool _WriteToINI(CCINIClass* pINI);

}; static_assert(sizeof(FakTaskForceClass) == sizeof(TaskForceClass), "Invalid Size !");