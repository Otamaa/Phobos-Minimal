#pragma once

#include <Memory.h>

//call gamedelete functuion without the DTOR
template<typename T>
struct UniqueGamePtr : public std::unique_ptr<T, GameDeleterWithDTOR> {

	COMPILETIMEEVAL UniqueGamePtr<T>() noexcept : std::unique_ptr<T, GameDeleterWithDTOR>()
	{ }

	COMPILETIMEEVAL UniqueGamePtr<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDeleterWithDTOR>(){
		this->reset(_ptr);
	}

	UniqueGamePtr(std::unique_ptr<T>) = delete;
	template<typename U = T, typename D = GameDeleterWithDTOR>
	static std::unique_ptr<U, D> Make(U* ptr)
	{
		static_assert(std::is_same_v<D, GameDeleterWithDTOR>,
			"Only GameDeleterWithDTOR is allowed. Use UniqueGamePtr instead of std::unique_ptr.");

		return std::unique_ptr<U, D>(ptr);
	}
};

// //call only the DTOR
// template<typename T>
// struct UniqueGamePtrB : public std::unique_ptr<T, GameDTORCaller>{
//
// 	COMPILETIMEEVAL UniqueGamePtrB<T>() noexcept : std::unique_ptr<T, GameDTORCaller>()
// 	{ }
//
// 	COMPILETIMEEVAL UniqueGamePtrB<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDTORCaller>() {
// 		this->reset(_ptr);
// 	}
// };

//call gamedelete functuion with the DTOR
// template<typename T>
// struct UniqueGamePtrC : public std::unique_ptr<T, GameDeleterWithDTOR>{
//
// 	COMPILETIMEEVAL UniqueGamePtrC<T>() noexcept : std::unique_ptr<T, GameDeleterWithDTOR>()
// 	{ }
//
// 	COMPILETIMEEVAL UniqueGamePtrC<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDeleterWithDTOR>() {
// 		this->reset(_ptr);
// 	}
//
// 	UniqueGamePtrC(std::unique_ptr<T>) = delete;
// 	template<typename U = T, typename D = GameDeleterWithDTOR>
// 	static std::unique_ptr<U, D> Make(U* ptr)
// 	{
// 		static_assert(std::is_same_v<D, GameDeleterWithDTOR>,
// 			"Only GameDeleterWithDTOR is allowed. Use UniqueGamePtrC instead of std::unique_ptr.");
//
// 		return std::unique_ptr<U, D>(ptr);
// 	}
// };

