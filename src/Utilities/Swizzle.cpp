#include "Swizzle.h"

#include <Phobos.h>

#include "Debug.h"

#include <SwizzleManagerClass.h>

#ifndef  DISABLE_ARES_SWIZZLEHOOKS
PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::RegisterForChange(void** p)
{
	return SwizzleManagerClass::Instance().Swizzle(p);
}

HRESULT PhobosSwizzle::RegisterChange(void* was, void* is)
{
	return SwizzleManagerClass::Instance().Here_I_Am((long)was, is);
}
#else
PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::RegisterForChange(void** p)
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

HRESULT PhobosSwizzle::RegisterChange(void* was, void* is)
{
	auto exist = this->Changes.find(was);
	if (exist == this->Changes.end())
	{
		this->Changes[was] = is;
	}
	else if (exist->second != is)
	{
		Debug::Log("PhobosSwizze :: Pointer [%p] declared change to both [%p] AND [%p]!\n", was, exist->second, is);
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

DEFINE_HOOK(0x6CF350, SwizzleManagerClass_ConvertNodes, 0x0)
{
	PhobosSwizzle::Instance.ConvertNodes();
	PhobosSwizzle::Instance.Clear();

	return 0x6CF400;
}

DEFINE_HOOK(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 0x0)
{
	GET_STACK(void*, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);

	HRESULT res = PhobosSwizzle::Instance.RegisterChange(oldP, newP);

	R->EAX<HRESULT>(res);
	return 0x6CF316;
}

DEFINE_HOOK(0x6CF240, SwizzleManagerClass_Swizzle, 0x0)
{
	GET_STACK(void**, ptr, 0x8);

	HRESULT res = PhobosSwizzle::Instance.RegisterForChange(ptr);

	R->EAX<HRESULT>(res);
	return 0x6CF2B3;
}
#endif