#pragma once

// Ares has hooked the SwizzleManagerClass,
// so what we need to do is just call the original functions.

#include <unordered_map>
#include <type_traits>

#include <Objidl.h>

class PhobosSwizzle
{
protected:

	/**
	* data store for RegisterChange
	*/
	std::unordered_map<void*, void*> Changes;

	/**
	* data store for RegisterForChange
	*/
	std::unordered_multimap<void*, void**> Nodes;

public:
	static PhobosSwizzle Instance;

	PhobosSwizzle() { }
	~PhobosSwizzle() { }

	/**
	* pass in the *address* of the pointer you want to have changed
	* caution, after the call *p will be NULL
	*/
	HRESULT RegisterForChange(void** p);

	/**
	* the original game objects all save their `this` pointer to the save stream
	* that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
	*/
	HRESULT RegisterChange(void* was, void* is);

	/**
	* this function will rewrite all registered nodes' values
	*/
	void ConvertNodes() const;

	void Clear();

	template<typename T>
	void RegisterPointerForChange(T*& ptr)
	{
		auto pptr = const_cast<std::remove_cv_t<T>**>(&ptr);
		this->RegisterForChange(reinterpret_cast<void**>(pptr));
	}
};

template<typename T>
struct is_swizzlable : public std::is_pointer<T>::type { };

struct Swizzle {
	template <typename T>
	Swizzle(T& object)
	{
		swizzle(object, typename is_swizzlable<T>::type());
	}

private:
	template <typename TSwizzle>
	void swizzle(TSwizzle& object, std::true_type)
	{
		PhobosSwizzle::Instance.RegisterPointerForChange(object);
	}

	template <typename TSwizzle>
	void swizzle(TSwizzle& object, std::false_type)
	{
		//Debug::Log("%s Is Not Swizzeable ! \n", typeid(TSwizzle).name());
	}
};
