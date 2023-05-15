#pragma once

#include <Locomotor/Cast.h>
#include <FootClass.h>

template <class T, bool CheckInterface = false>
FORCEINLINE T GetLocomotor(FootClass* pThis)
{
	static_assert(std::is_pointer<T>::value, "T Required To Be Pointer !");

	// yikes
	if constexpr (!std::is_const<T>())
	{
		static_assert(std::is_base_of<ILocomotion, std::remove_pointer_t<T>>::value,
		"GetLocomotor: T is required to be a type derived from ILocomotion.");

		static_assert(std::is_base_of<LocomotionClass, std::remove_pointer_t<T>>::value,
		"GetLocomotor: T is required to be a type derived from LocomotionClass.");
	}
	else
	{
		static_assert(std::is_base_of<ILocomotion, std::remove_const_t<std::remove_pointer_t<T>>>::value,
		"GetLocomotor: T is required to be a type derived from ILocomotion.");

		static_assert(std::is_base_of<LocomotionClass, std::remove_const_t<std::remove_pointer_t<T>>>::value,
		"GetLocomotor: T is required to be a type derived from LocomotionClass.");
	}

	if constexpr (!CheckInterface)
		return static_cast<T>(pThis->Locomotor.get());
	else
		return static_cast<T>(pThis->Locomotor);

}

template <class JumpjetLocomotionClass , bool CheckInterface>
JumpjetLocomotionClass* GetLocomotorType(FootClass* pThis)
{
	const auto pILoco = GetLocomotor<JumpjetLocomotionClass* , CheckInterface>(pThis);
	return locomotion_cast<JumpjetLocomotionClass*>(pILoco);
}