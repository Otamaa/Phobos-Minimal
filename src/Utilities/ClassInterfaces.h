#pragma once

#include <type_traits>

class CCINIClass;
class PhobosStreamReader;
class PhobosStreamWriter;

template<typename T>
struct ReadWriteContainerInterfaces
{
	virtual void LoadFromINI(T::base_type* key, CCINIClass* pINI, bool parseFailAddr) = 0;
	virtual void WriteToINI(T::base_type* key, CCINIClass* pINI) = 0;
};

struct GlobalSaveable
{
	virtual bool SaveGlobal(PhobosStreamWriter& stm) = 0;
	virtual bool LoadGlobal(PhobosStreamReader& stm) = 0;
	virtual void Clear() = 0;
};

struct ContainerInterfaces
{
	virtual void Clear() = 0;
	virtual void ClearNullAttachedObj() = 0;
};