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

	HRESULT Swizzle(void** p);
	HRESULT Here_I_Am(void* was, void* is, bool log = true);

	void ConvertNodes() const;

	FORCEDINLINE void Clear()
	{
		this->Nodes.clear();
		this->Changes.clear();
	}

	template<typename T>
	FORCEDINLINE void RegisterPointerForChange(T*& ptr) {
		this->Swizzle(reinterpret_cast<void**>(const_cast<std::remove_cv_t<T>**>(&ptr)));
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
