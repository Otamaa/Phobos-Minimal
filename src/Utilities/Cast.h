#pragma once
#include "Concepts.h"
#include <TechnoTypeClass.h>

template<typename T>
struct type_cast_impl
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	inline T operator()(const ObjectTypeClass* pAbstract) {

		/*Added to TechnoTypeClass , ObjectTypeClass , AbstractTypeClass:
		*These Information is just for compile time
		*The result will be inlined (/O2 /Ob2)
		enum class AbstractBaseType : int
		{
			Root = 0,
			ObjectType,
			TechnoType
		};
		*/

		return Handle(pAbstract, std::bool_constant<Base::AbsID == AbstractType::Abstract && Base::AbsTypeBase == AbstractBaseType::TechnoType>::type());
	}

private:
	T Handle(const ObjectTypeClass* pAbstract, std::true_type) noexcept {
		return pAbstract->WhatAmI() == AbstractType::AircraftType
			|| pAbstract->WhatAmI() == AbstractType::BuildingType
			|| pAbstract->WhatAmI() == AbstractType::InfantryType
			|| pAbstract->WhatAmI() == AbstractType::UnitType
			 ? static_cast<T>(pAbstract) : nullptr;
	}

	T Handle(const ObjectTypeClass* pAbstract, std::false_type) noexcept {
		return Handle_FalseType(pAbstract, std::bool_constant<Base::AbsID == AbstractType::Abstract &&
			(Base::AbsTypeBase == AbstractBaseType::ObjectType)>::type());
	}

	T Handle_FalseType(const ObjectTypeClass* pAbstract, std::true_type) noexcept {
		return pAbstract->WhatAmI() == AbstractType::AircraftType
			|| pAbstract->WhatAmI() == AbstractType::AnimType
			|| pAbstract->WhatAmI() == AbstractType::BuildingType
			|| pAbstract->WhatAmI() == AbstractType::BulletType
			|| pAbstract->WhatAmI() == AbstractType::InfantryType
			|| pAbstract->WhatAmI() == AbstractType::IsotileType
			|| pAbstract->WhatAmI() == AbstractType::UnitType
			|| pAbstract->WhatAmI() == AbstractType::OverlayType
			|| pAbstract->WhatAmI() == AbstractType::ParticleType
			|| pAbstract->WhatAmI() == AbstractType::ParticleSystemType
			|| pAbstract->WhatAmI() == AbstractType::SmudgeType
			|| pAbstract->WhatAmI() == AbstractType::TerrainType
			|| pAbstract->WhatAmI() == AbstractType::VoxelAnimType
			? static_cast<T>(pAbstract) : nullptr;
	}

	T Handle_FalseType(const ObjectTypeClass* pAbstract, std::false_type) noexcept {
		return pAbstract->WhatAmI() == Base::AbsID ? static_cast<T>(pAbstract) : nullptr;
	}
};

template <typename T>
inline T type_cast(ObjectTypeClass* pAbstract)
{
	using Base = std::remove_pointer_t<T>;
	return const_cast<Base*>(type_cast<const Base*>(static_cast<const ObjectTypeClass*>(pAbstract)));
};

template <typename T>
inline T type_cast(const ObjectTypeClass* pAbstract)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	static_assert(std::is_const<std::remove_pointer_t<T>>::value,
	"type_cast: T is required to be const.");

	static_assert(std::is_base_of<ObjectTypeClass, Base>::value,
	"type_cast: T is required to be a type derived from ObjectTypeClass.");

	static_assert(!std::bool_constant<Base::AbsID == AbstractType::Abstract && Base::AbsTypeBase == AbstractBaseType::Root>::value,
	"type_cast: T from AbstractTypeClass is unsupported.");

	return pAbstract ? type_cast_impl<T>()(pAbstract) : nullptr;
}
