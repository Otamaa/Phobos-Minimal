#pragma once

#include <AbstractClass.h>

#include <type_traits>

class ObjectClass;
class MissionClass;
class RadioClass;
class TechnoClass;
class FootClass;

template <typename T>
FORCEINLINE T cast_to(AbstractClass* pAbstract) {
	using Base = std::remove_pointer_t<T>;

	return const_cast<Base*>(cast_to<const Base*>(static_cast<const AbstractClass*>(pAbstract)));
};

template <typename T>
FORCEINLINE T cast_to(const AbstractClass* pAbstract) {
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value,
		"specific_cast: T is required to be const.");

	static_assert(std::is_base_of<AbstractClass, Base>::value,
		"specific_cast: T is required to be a type derived from AbstractClass.");

	static_assert(!std::is_abstract<Base>::value,
		"specific_cast: Abstract types (not fully implemented classes) are not suppored.");

	return pAbstract && pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
};

template <typename T>
FORCEINLINE T flag_cast_to(const AbstractClass* pAbstract) {
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value,
		"generic_cast: T is required to be const.");

	static_assert(std::is_base_of<ObjectClass, Base>::value
		//&& std::is_abstract<Base>::value
		,"generic_cast: T is required to be an abstract type derived from ObjectClass.");

	return (pAbstract && (pAbstract->AbstractFlags & Base::AbsDerivateID) != AbstractFlags::None)  ? static_cast<T>(pAbstract) : nullptr;
};

template <typename T>
FORCEINLINE T flag_cast_to(AbstractClass* pAbstract) {
	using Base = std::remove_pointer_t<T>;

	return const_cast<T>(flag_cast_to<const Base*>(static_cast<const AbstractClass*>(pAbstract)));
};