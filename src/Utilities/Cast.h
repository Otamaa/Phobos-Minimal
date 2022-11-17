#pragma once
#include "Concepts.h"
#include <TechnoTypeClass.h>

struct type_cast_data
{
	static char BytesData[76];
};

template<typename T>
struct type_cast_impl
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static bool IsTechnoType(const AbstractType key) noexcept
	{
		return ((type_cast_data::BytesData[(int)key] & 4) != 0);
	}

	static bool IsObjectType(const AbstractType key) noexcept
	{
		return ((type_cast_data::BytesData[(int)key] & 2) != 0);
	}

	NOINLINE T operator()(const ObjectTypeClass* pAbstract) noexcept
	{
		if constexpr (Base::AbsID == AbstractType::Abstract)
		{
			if constexpr (Base::AbsTypeBase == AbstractBaseType::TechnoType)
			{
				return IsTechnoType(pAbstract->WhatAmI())
					? static_cast<T>(pAbstract) : nullptr;
			}
			else
			if constexpr (Base::AbsTypeBase == AbstractBaseType::ObjectType)
			{
				return IsObjectType(pAbstract->WhatAmI()) ? static_cast<T>(pAbstract) : nullptr;
			} else {
				return pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
			}

		} else { 
			return pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
		}
	}
};

template <typename T ,bool Check = true>
inline T type_cast(ObjectTypeClass* pAbstract)
{
	using Base = std::remove_pointer_t<T>;
	return const_cast<Base*>(type_cast<const Base*, Check>(static_cast<const ObjectTypeClass*>(pAbstract)));
};

template <typename T, bool Check = true>
NOINLINE T type_cast(const ObjectTypeClass* pAbstract)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value,
	"type_cast: T is required to be const.");

	static_assert(std::is_base_of<ObjectTypeClass, Base>::value,
	"type_cast: T is required to be a type derived from ObjectTypeClass.");

	static_assert(!std::bool_constant<Base::AbsID == AbstractType::Abstract && Base::AbsTypeBase == AbstractBaseType::Root>::value,
	"type_cast: T from AbstractTypeClass is unsupported.");

	if constexpr (Check)
		return pAbstract ? type_cast_impl<T>()(pAbstract) : nullptr;
	else
		return type_cast_impl<T>()(pAbstract);
}
