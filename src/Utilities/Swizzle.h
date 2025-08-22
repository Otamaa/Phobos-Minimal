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

	HRESULT RegisterForFixup(void** p);
	HRESULT DeclareMapping(void* was, void* is, bool log = true);

	void ApplyFixups() const;

	FORCEDINLINE void Clear()
	{
		this->Nodes.clear();
		this->Changes.clear();
	}

	template<typename T>
	FORCEDINLINE void RegisterPointerForChange(T*& ptr)
	{
		// Already a pointer, just forward
		this->RegisterForFixup(reinterpret_cast<void**>(
			const_cast<std::remove_cv_t<T>**>(&ptr)
			));
	}

	template<typename T>
	FORCEDINLINE void RegisterPointerForChange(T& obj)
	{
		// Non-pointer object: take its address
		auto* ptr = &obj;
		this->RegisterPointerForChange(ptr);
	}
};

template<typename T>
concept is_swizzlable = std::is_pointer<T>::value;

struct Swizzle {
	template <typename T>
	Swizzle(T& object) {
		if COMPILETIMEEVAL(is_swizzlable<T>)
			PhobosSwizzle::Instance.RegisterPointerForChange(object);
	}
};
