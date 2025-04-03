#pragma once
#include <type_traits>
#include <Base/Always.h>

// IMO under no circumstances should we set the vtable address for a class from vanilla game in general,
// we do this only for pointers
// For the addresses declare them in their class, not here. Pay attention to templated ones like DVC
namespace VTable
{
	template<typename T>
	COMPILETIMEEVAL OPTIONALINLINE void Set(const T ptr, uintptr_t addr, size_t offset = 0)
	{
		static_assert(std::is_pointer<T>::value, "T must be a pointer");
		reinterpret_cast<uintptr_t*>(ptr)[offset] = addr;
	}

	template<typename T>
	COMPILETIMEEVAL OPTIONALINLINE uintptr_t Get(const T ptr, size_t offset = 0)
	{
		static_assert(std::is_pointer<T>::value, "T must be a pointer");

		if COMPILETIMEEVAL (std::is_volatile_v<std::remove_pointer_t<T>> || std::is_const_v<std::remove_pointer_t<T>>)
			return reinterpret_cast<const volatile uintptr_t*>(ptr)[offset];
		else
			return reinterpret_cast<uintptr_t*>(ptr)[offset];
	}
}
// Not sure if correct for all cases yet
