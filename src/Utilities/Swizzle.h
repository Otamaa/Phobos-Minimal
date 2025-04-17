#pragma once

#include <vector>
#include <type_traits>

#include <Objidl.h>
#include <SwizzleManagerClass.h>

#include <Utilities/Debug.h>

#include <unordered_map>

class PhobosSwizzle
{
public:

	std::unordered_map<void*, void*> Changes {};
	std::unordered_multimap<void*, void**> Nodes {};

public:
	static PhobosSwizzle Instance;

	FORCEDINLINE HRESULT RegisterForChange(void** p)
	{
		if (p) {
			if (auto deref = *p) {
				this->Nodes.emplace(deref, p);
				*p = nullptr;
			}

			return S_OK;
		}
		return E_POINTER;
	}

	FORCEDINLINE HRESULT RegisterChange(void* was, void* is)
	{
		auto exist = this->Changes.find(was);

		//the requested `was` not found
		if (exist == this->Changes.end()) {
			Debug::LogInfo("PhobosSwizze :: Pointer [{}] request change to [{}]!", was, is);
			this->Changes.emplace(was, is);
		}
		//the requested `was` found
		else if (exist->second != is) {
			Debug::LogInfo("PhobosSwizze :: Pointer [{}] are declared change to both [{}] AND [{}]!", was, exist->second, is);
		}

		return S_OK;
	}

	void ConvertNodes() const
	{
		Debug::LogInfo("PhobosSwizze :: Converting {} nodes.", this->Nodes.size());
		void* lastFind(nullptr);
		void* lastRes(nullptr);

		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it) {
			if (lastFind != it->first) {
				const auto change = this->Changes.find(it->first);

				if (change == this->Changes.end()) {
					Debug::LogInfo("PhobosSwizze :: Pointer [{}] could not be remapped from [{}] !", (void*)(it->second), (void*)(it->first));
				} else {
					lastFind = it->first;
					lastRes = change->second;
				}
			}

			if (auto p = it->second) {
				*p = lastRes;
			}

		}
	}

	FORCEDINLINE void Clear()
	{
		this->Nodes.clear();
		this->Changes.clear();
	}

	template<typename T>
	FORCEDINLINE void RegisterPointerForChange(T*& ptr) {
		this->RegisterForChange(reinterpret_cast<void**>(const_cast<std::remove_cv_t<T>**>(&ptr)));
	}
};

template<typename T>
struct is_swizzlable : public std::is_pointer<T>::type { };

struct Swizzle {
	template <typename T>
	Swizzle(T& object) {
		if COMPILETIMEEVAL (std::is_pointer_v<T>) {
			PhobosSwizzle::Instance.RegisterPointerForChange(object);
		}
#ifdef _DEBUG
		else {
			Debug::LogInfo("{} Is Not Swizzeable ! ", typeid(T).name());
		}
#endif
	}
};
