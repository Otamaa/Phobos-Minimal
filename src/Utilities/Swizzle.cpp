#include "Swizzle.h"

PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::Swizzle(void** p)
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

HRESULT PhobosSwizzle::Here_I_Am(void* was, void* is, bool log)
{
	auto exist = this->Changes.find(was);

	//the requested `was` not found
	if (exist == this->Changes.end())
	{
		if(log)
			Debug::LogInfo("PhobosSwizze :: Pointer [{}] request change to [{}]!", was, is);

		this->Changes.emplace(was, is);
	}
	//the requested `was` found
	else if (exist->second != is)
	{
		if (log)
			Debug::LogInfo("PhobosSwizze :: Pointer [{}] are declared change to both [{}] AND [{}]!", was, exist->second, is);
	}

	return S_OK;
}

void PhobosSwizzle::ConvertNodes() const
{
	Debug::LogInfo("PhobosSwizze :: Converting {} nodes.", this->Nodes.size());

#ifndef _old
	void* lastFind(nullptr);
	void* lastRes(nullptr);

	for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it)
	{
		if (lastFind != it->first)
		{
			const auto change = this->Changes.find(it->first);

			if (change == this->Changes.end())
			{
				Debug::Log("PhobosSwizze :: Pointer [%x] could not be remapped from [%x] !", (void*)(it->second), (void*)(it->first));
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
#else

	//these break the radar
	for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it)
	{
		const auto change = this->Changes.find(it->first);

		if (!it->second || change == this->Changes.end())
		{
			Debug::LogInfo("PhobosSwizze :: Pointer [{}] could not be remapped from [{}] !", (void*)(it->second), (void*)(it->first));
		}
		else
		{
			*it->second = change->second;
		}
	}

#endif
}