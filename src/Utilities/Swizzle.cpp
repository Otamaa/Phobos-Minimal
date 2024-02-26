#include "Swizzle.h"

#include <Phobos.h>

#include "Debug.h"

#include <SwizzleManagerClass.h>

HRESULT PhobosSwizzle::RegisterForChange(void** p)
{
	return SwizzleManagerClass::Instance().Swizzle(p);
}

HRESULT PhobosSwizzle::RegisterChange(void* was, void* is)
{
	return SwizzleManagerClass::Instance().Here_I_Am((long)was, is);
}

PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::RegisterForChange_Hook(void** p)
{
	if (p)
	{
		if (auto deref = *p)
		{
			this->Nodes.emplace(deref, p);
			*p = nullptr;
		}
		return S_OK;
	}
	return E_POINTER;
}

HRESULT PhobosSwizzle::RegisterChange_Hook(DWORD caller , void* was, void* is)
{
	auto exist = this->Changes.find(was);
	//the requested `was` not found
	if (exist == this->Changes.end())
	{
		Debug::Log("PhobosSwizze[0x%x] :: Pointer [%p] request change to both [%p] AND [%p]!\n", caller, was, exist->second, is);

		this->Changes[was] = is;
	}
	//the requested `was` found
	else if (exist->second != is)
	{
		Debug::Log("PhobosSwizze[0x%x] :: Pointer [%p] declared change to both [%p] AND [%p]!\n",caller, was, exist->second, is);
	}
	return S_OK;
}

void PhobosSwizzle::ConvertNodes() const
{
	Debug::Log("PhobosSwizze :: Converting %u nodes.\n", this->Nodes.size());
	void* lastFind(nullptr);
	void* lastRes(nullptr);
	for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it)
	{
		if (lastFind != it->first)
		{
			auto change = this->Changes.find(it->first);
			if (change == this->Changes.end())
			{
				Debug::Log("PhobosSwizze :: Pointer [%p] could not be remapped from [%p] !\n", it->second , it->first );
			}
			else
			{
				lastFind = it->first;
				lastRes = change->second;
			}
		}
		if (auto p = it->second)
		{
			*p = lastRes;
		}
	}
}

void PhobosSwizzle::Clear()
{
	this->Nodes.clear();
	this->Changes.clear();
}