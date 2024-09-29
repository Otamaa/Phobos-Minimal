#pragma once

#include <vector>
#include <type_traits>

#include <Objidl.h>
#include <SwizzleManagerClass.h>

#include <Utilities/Debug.h>

class PhobosSwizzle
{
protected:

	/**
	* data store for RegisterChange
	*/
	std::vector<std::pair<void*, void*>> Changes {};

	/**
	* data store for RegisterForChange
	*/
	std::vector<std::pair<void*, void**>> Nodes  {};

public:
	static PhobosSwizzle Instance;

	/**
	* pass in the *address* of the pointer you want to have changed
	* caution, after the call *p will be NULL
	*/
	FORCEINLINE constexpr HRESULT RegisterForChange_Hook(void** p)
	{
		if (p)
		{
			if (auto deref = *p)
			{
				this->Nodes.emplace_back(deref, p);
				*p = nullptr;
			}
			return S_OK;
		}
		return E_POINTER;
	}

	FORCEINLINE HRESULT RegisterForChange(void** p)
	{
		return SwizzleManagerClass::Instance().Swizzle(p);
	}

	FORCEINLINE constexpr auto FindChanges(void* ptr) const {

		for (auto begin = this->Changes.begin(); begin != this->Changes.end(); ++begin) {
			if (begin->first == ptr)
				return begin;
		}

		return this->Changes.end();
	}

	/**
	* the original game objects all save their `this` pointer to the save stream
	* that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
	*/
	FORCEINLINE HRESULT RegisterChange(void* was, void* is)
	{
		return SwizzleManagerClass::Instance().Here_I_Am((long)was, is);
	}

	FORCEINLINE constexpr HRESULT RegisterChange_Hook(DWORD caller , void* was, void* is)
	{
		auto exist = this->FindChanges(was);

		//the requested `was` not found
		if (exist == this->Changes.end()) {
			//Debug::Log("PhobosSwizze[0x%x] :: Pointer [%p] request change to both [%p] AND [%p]!\n", caller, was, exist->second, is);
			this->Changes.emplace_back(was, is);
		}
		//the requested `was` found
		//else if (exist->second != is) {
		//	Debug::Log("PhobosSwizze[0x%x] :: Pointer [%p] declared change to both [%p] AND [%p]!\n", caller, was, exist->second, is);
		//}

		return S_OK;
	}

	/**
	* this function will rewrite all registered nodes' values
	*/

	//constexpr , removed since need to log ,..
	// probably make constexpr Debug::Log
		void ConvertNodes() const
	{
		//Debug::Log("PhobosSwizze :: Converting %u nodes.\n", this->Nodes.size());
		void* lastFind(nullptr);
		void* lastRes(nullptr);

		for (auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it)
		{
			if (lastFind != it->first)
			{
				const auto change = this->FindChanges(it->first);

				if (change == this->Changes.end()) {
					Debug::Log("PhobosSwizze :: Pointer [%p] could not be remapped from [%p] !\n", it->second, it->first);
				} else //if (change != this->Changes.end())
				{
					lastFind = it->first;
					lastRes = change->second;
				}
			}

			if (auto p = it->second) {
				*p = lastRes;
			}

		}
	}

	constexpr FORCEINLINE void Clear()
	{
		this->Nodes.clear();
		this->Changes.clear();
	}

	template<typename T>
	FORCEINLINE void RegisterPointerForChange(T*& ptr) {
		this->RegisterForChange(reinterpret_cast<void**>(const_cast<std::remove_cv_t<T>**>(&ptr)));
	}
};

template<typename T>
struct is_swizzlable : public std::is_pointer<T>::type { };

struct Swizzle {
	template <typename T>
	Swizzle(T& object) {
		if constexpr (std::is_pointer_v<T>) {
			PhobosSwizzle::Instance.RegisterPointerForChange(object);
		}
#ifdef _DEBUG
		else {
			Debug::Log("%s Is Not Swizzeable ! \n", typeid(TSwizzle).name());
		}
#endif
	}
};
