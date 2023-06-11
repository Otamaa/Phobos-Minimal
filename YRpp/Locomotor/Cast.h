#pragma once

#include "DriveLocomotionClass.h" 
#include "DropPodLocomotionClass.h"
#include "FlyLocomotionClass.h"
#include "JumpjetLocomotionClass.h"
#include "HoverLocomotionClass.h"
#include "RocketLocomotionClass.h"
#include "MechLocomotionClass.h"
#include "ShipLocomotionClass.h"
#include "TeleportLocomotionClass.h"
#include "TunnelLocomotionClass.h"
#include "WalkLocomotionClass.h"

template<typename T>
concept LocoHasILocoVtbl = std::is_base_of_v<LocomotionClass, std::remove_cvref_t<T>>&& requires
{
	{ T::ILoco_vtable }->std::convertible_to<uintptr_t>;
};

template<typename T>
concept LocoHasClassGUID = std::is_base_of_v<LocomotionClass, std::remove_cvref_t<T>>&&
std::is_same_v<std::remove_reference_t<decltype(T::ClassGUID.get())>, CLSID>;

template <typename T, bool check = false>
__forceinline T locomotion_cast(ILocomotion* iLoco)
{
	static_assert(std::is_pointer<T>::value, "locomotion_cast: Pointer is required.");
	using Base = std::remove_cvref_t<std::remove_const_t<std::remove_pointer_t<T>>>;
	static_assert(std::is_base_of_v<LocomotionClass, Base> && !std::is_same_v<LocomotionClass, Base>,
		"T needs to point to a class derived from LocomotionClass");

	if constexpr (check){
		if (!iLoco)
			return (T)nullptr;
	}

	if constexpr (LocoHasILocoVtbl<Base>) {
		return (VTable::Get(iLoco) == Base::ILoco_vtable) ? static_cast<T>(iLoco) : nullptr;
	} 
	else if constexpr (LocoHasClassGUID<Base>) 
	{
		CLSID locoCLSID;
		return (SUCCEEDED(static_cast<LocomotionClass*>(iLoco)->GetClassID(&locoCLSID)) && locoCLSID == Base::ClassGUID()) ?
			static_cast<T>(iLoco) : nullptr;
	}
}

template<typename T , bool check = false>
__forceinline T locomotion_cast(ILocomotionPtr& comLoco)
{
	static_assert(std::is_pointer<T>::value, "locomotion_cast: Pointer is required.");
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	static_assert(std::is_base_of<LocomotionClass, Base>::value,
		"locomotion_cast: T is required to be a sub-class of LocomotionClass.");

	return locomotion_cast<T, check>(comLoco.GetInterfacePtr());
}