#pragma once

#include <LocomotionClass.h>
#include <FootClass.h>
#include <JumpjetLocomotionClass.h>

template <class T, bool CheckInterface = false>
FORCEINLINE T GetLocomotor(FootClass* pThis)
{
	static_assert(std::is_pointer<T>(), "T Required To Be Pointer !");

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

template <class T , bool CheckInterface = false>
FORCEINLINE T* GetLocomotorType(FootClass* pThis)
{
	CLSID locoCLSID {};
	const auto pILoco = GetLocomotor<T* , CheckInterface>(pThis);

	//it is already return as JumJetLoco on the top , but we check the CLSID here to make sure
	//we got real T* pointer instead of something else
	return //(SUCCEEDED(pILoco->GetClassID(&locoCLSID)) && locoCLSID == __uuidof(T)) ?
		(((int*)pILoco)[0] == 0x7ECE34) ? //faster
			static_cast<T*>(pILoco) : nullptr;
}