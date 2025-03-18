#pragma once

#include <Memory.h>

//call gamedelete functuion without the DTOR
template<typename T>
struct UniqueGamePtr : public std::unique_ptr<T, GameDeleter> {

	COMPILETIMEEVAL UniqueGamePtr<T>() noexcept : std::unique_ptr<T, GameDeleter>()
	{ }

	COMPILETIMEEVAL UniqueGamePtr<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDeleter>(){
		this->reset(_ptr);
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
template<typename T>
struct UniqueGamePtrC : public std::unique_ptr<T, GameDeleterWithDTOR>{

	COMPILETIMEEVAL UniqueGamePtrC<T>() noexcept : std::unique_ptr<T, GameDeleterWithDTOR>()
	{ }

	COMPILETIMEEVAL UniqueGamePtrC<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDeleterWithDTOR>() {
		this->reset(_ptr);
	}
};

