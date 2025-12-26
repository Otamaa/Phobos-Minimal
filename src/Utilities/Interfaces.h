#include <Phobos.h>

#pragma once

class CCINIClass;

template<typename T>
struct ReadWriteContainerInterfaces
{
	virtual void LoadFromINI(T::base_type* key, CCINIClass* pINI, bool parseFailAddr) = 0;
	virtual void WriteToINI(T::base_type* key, CCINIClass* pINI) = 0;
};

struct GlobalSaveable
{
	virtual bool SaveGlobal(json& root) = 0;
	virtual bool LoadGlobal(const json& root) = 0;
	virtual void Clear() = 0;
};

struct ContainerInterfaces
{
	virtual bool LoadAll(const json& root) = 0;
	virtual bool SaveAll(json& root) = 0;
	virtual void Clear() = 0;
};
