#pragma once
#include "Concepts.h"
#include <TechnoTypeClass.h>

template<typename T>
struct type_cast_impl
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	inline T operator()(const ObjectTypeClass* pAbstract)
	{
		if constexpr (Base::AbsID == AbstractType::Abstract)
		{
			if constexpr (Base::AbsTypeBase == AbstractBaseType::TechnoType)
			{
				auto const pWhat = pAbstract->WhatAmI();
				return pWhat == AbstractType::AircraftType
					|| pWhat == AbstractType::BuildingType
					|| pWhat == AbstractType::InfantryType
					|| pWhat == AbstractType::UnitType
					? static_cast<T>(pAbstract) : nullptr;
			}
			else
			if constexpr (Base::AbsTypeBase == AbstractBaseType::ObjectType)
			{
				auto const pWhat = pAbstract->WhatAmI();
				return pWhat == AbstractType::AircraftType
					|| pWhat == AbstractType::AnimType
					|| pWhat == AbstractType::BuildingType
					|| pWhat == AbstractType::BulletType
					|| pWhat == AbstractType::InfantryType
					|| pWhat == AbstractType::IsotileType
					|| pWhat == AbstractType::UnitType
					|| pWhat == AbstractType::OverlayType
					|| pWhat == AbstractType::ParticleType
					|| pWhat == AbstractType::ParticleSystemType
					|| pWhat == AbstractType::SmudgeType
					|| pWhat == AbstractType::TerrainType
					|| pWhat == AbstractType::VoxelAnimType
					? static_cast<T>(pAbstract) : nullptr;
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
inline T type_cast(const ObjectTypeClass* pAbstract)
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
