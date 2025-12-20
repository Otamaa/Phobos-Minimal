#pragma once

#include <AbstractClass.h>

#include <type_traits>

class ObjectClass;
class MissionClass;
class RadioClass;
class TechnoClass;
class FootClass;

template<typename T>
concept HasAbsTypeBase =
	requires(T)
{
	T::AbsTypeBase;
};

template <typename T>
concept abs_id_is_abstract_type = std::is_same_v<std::remove_cv_t<decltype(T::AbsID)>, AbstractType>
&& (T::AbsID != AbstractType::Abstract);

// pAbs->WhatAmI() == Base::AbsID check
template <typename T , bool check = true>
FORCEDINLINE T cast_to(AbstractClass* pAbstract) {
	using Base = std::remove_pointer_t<T>;

	return const_cast<Base*>(cast_to<const Base*, check>(static_cast<const AbstractClass*>(pAbstract)));
};

// pAbs->WhatAmI() == Base::AbsID check
template <typename T, bool check = true>
FORCEDINLINE T cast_to(const AbstractClass* pAbstract) {
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value,
		"cast_to: T is required to be const.");

	static_assert(std::is_base_of<AbstractClass, Base>::value,
		"cast_to: T is required to be a type derived from AbstractClass.");

	static_assert(!std::is_abstract<Base>::value,
		"cast_to: Abstract types (not fully implemented classes) are not suppored.");

	static_assert(abs_id_is_abstract_type<Base>,
		  "cast_to: T casting from AbsDerivateID will produce wrong result.");

	static_assert(!HasAbsTypeBase<Base>,
		  "cast_to: T casting from AbsTypeBase will produce wrong result.");

	if COMPILETIMEEVAL (check)
		return pAbstract && pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
	else
		return pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
};

// pAbs->AbstractFlags & Base::AbsDerivateID check
template <typename T, bool check = true>
FORCEDINLINE T flag_cast_to(const AbstractClass* pAbstract) {
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value
		, "flag_cast_to: T is required to be const.");

	static_assert(std::is_base_of<ObjectClass, Base>::value
		, "flag_cast_to: T is required to be an abstract type derived from ObjectClass.");

	static_assert(!abs_id_is_abstract_type<Base>
		, "flag_cast_to: T casting from AbsID will produce wrong result.");

	static_assert(!HasAbsTypeBase<Base>
		, "flag_cast_to: T casting from AbsTypeBase will produce wrong result.");

	static_assert(Base::AbsDerivateID != AbstractFlags::None
		, "flag_cast_to: T casting from unknown Flags.");

	if COMPILETIMEEVAL (check)
		return (pAbstract && (pAbstract->AbstractFlags & Base::AbsDerivateID) != AbstractFlags::None)  ? static_cast<T>(pAbstract) : nullptr;
	else
		return (pAbstract->AbstractFlags & Base::AbsDerivateID) != AbstractFlags::None ? static_cast<T>(pAbstract) : nullptr;
};

// pAbs->pAbstract->AbstractFlags & Base::AbsDerivateID check
template <typename T, bool check = true>
FORCEDINLINE T flag_cast_to(AbstractClass* pAbstract) {
	using Base = std::remove_pointer_t<T>;

	return const_cast<T>(flag_cast_to<const Base*, check>(static_cast<const AbstractClass*>(pAbstract)));
};