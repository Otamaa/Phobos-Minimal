#pragma once

#include <Syringe.h>
#include <set>
#include <ArrayClasses.h>
#include <type_traits>
#include <Lib/fast_remove_if.h>

// here be dragons(plenty)

template<class, template<class...> class>
OPTIONALINLINE COMPILETIMEEVAL bool is_specialization = false;
template<template<class...> class T, class... Args>
OPTIONALINLINE COMPILETIMEEVAL bool is_specialization<T<Args...>, T> = true;

template<class T>
concept Vec = is_specialization<T, std::vector>;

template<class T>
concept NotAVec = !is_specialization<T, std::vector>;

// return functors

// set EAX to smth, return to smth
template<typename T>
class retfunc {
protected:
	REGISTERS *R;
	DWORD retAddr;
public:
	retfunc(REGISTERS *r, DWORD addr) : R(r), retAddr(addr) {};
	DWORD operator()(T Result) {
		R->EAX(Result);
		return retAddr;
	}
};

// set EAX to smth, return to fixed
template<typename T>
class retfunc_fixed : public retfunc<T> {
protected:
	T Result;
public:
	retfunc_fixed(REGISTERS *r, DWORD addr, T res) : retfunc<T>(r, addr), Result(res) {};
	DWORD operator()() {
		this->R->EAX(Result);
		return this->retAddr;
	}
};

// return to one of two fixed
class retfunc_bool : public retfunc<int> {
protected:
	DWORD negAddr;
public:
	retfunc_bool(REGISTERS *r, DWORD yAddr, DWORD nAddr) : retfunc<int>(r, yAddr), negAddr(nAddr) {};
	COMPILETIMEEVAL DWORD operator()(bool choose) {
		return choose ? retAddr : negAddr;
	}
};

// invalid pointers

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(T &elem, void *ptr) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	if(ptr == static_cast<void*>(elem)) {
		elem = nullptr;
	}
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(T& elem, void* ptr , bool removed) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	if (removed && ptr == static_cast<void*>(elem)) {
		elem = nullptr;
	}
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(DynamicVectorClass<T> &elem, void *ptr) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	elem.Remove((T)ptr);
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(DynamicVectorClass<T>& elem, void* ptr, bool removed) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	if(removed){
		elem.Remove((T)ptr);
	}
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(std::vector<T>& elem, void* ptr, bool removed) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	if (removed) {
		fast_remove_if(elem, [ptr](auto _el) { return  ptr == _el; });
	}
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(std::set<T>& elem, void* ptr, bool removed) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	if (removed){
		elem.erase((T)ptr);
	}
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(std::vector<T>& elem, void* ptr) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	fast_remove_if(elem, [ptr](auto _el) { return  ptr == _el; });
}

template<typename T>
void FORCEINLINE AnnounceInvalidPointer(std::set<T>& elem, void* ptr) {
	static_assert(std::is_pointer<T>::value, "Pointer Required !");
	elem.erase((T)ptr);
}

// vroom vroom
// Westwood uses if(((1 << HouseClass::ArrayIndex) & TechnoClass::DisplayProductionToHouses) != 0) and other bitfields like this (esp. in CellClass, omg optimized). helper wrapper just because
template <typename T>
class IndexBitfield {
public:
	COMPILETIMEEVAL IndexBitfield() = default;
	COMPILETIMEEVAL explicit IndexBitfield(DWORD const defVal) noexcept : data(defVal) {};

	COMPILETIMEEVAL IndexBitfield<T>& operator=(DWORD other)
	{
		std::swap(data, other);
		return *this;
	}

	COMPILETIMEEVAL bool Contains(const T obj) const {
		return Contains(obj->ArrayIndex);
	}

	COMPILETIMEEVAL bool Contains(int index) const {
		return (this->data & (1u << index)) != 0u;
	}

	COMPILETIMEEVAL void Add(int index) {
		this->data |= (1u << index);
	}

	COMPILETIMEEVAL void Add(const T obj) {
		Add(obj->ArrayIndex);
	}

	COMPILETIMEEVAL void Remove(int index) {
		this->data &= ~(1u << index);
	}

	COMPILETIMEEVAL void Remove(const T obj) {
		Remove(obj->ArrayIndex);
	}

	COMPILETIMEEVAL void Clear() {
		this->data = 0u;
	}

	COMPILETIMEEVAL operator DWORD() const {
		return this->data;
	}

	DWORD data{ 0u };
};

// template<class T, class EqualTo>
// struct has_operator_equal_impl
// {
// 	template<class U, class V>
// 	static auto test(U*) -> decltype(std::declval<U>() == std::declval<V>());
// 	template<typename, typename>
// 	static auto test(...) -> std::false_type;
//
// 	using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
// };

// template<class T, class EqualTo = T>
// struct has_operator_equal : has_operator_equal_impl<T, EqualTo>::type { };

#include <Helpers/Cast.h>
