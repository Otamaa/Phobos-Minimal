#pragma once

#include <Memory.h>

template<typename T>
struct UniqueGamePtr : public std::unique_ptr<T, GameDeleter> {

	COMPILETIMEEVAL UniqueGamePtr<T>() noexcept : std::unique_ptr<T, GameDeleter>()
	{ }

	COMPILETIMEEVAL UniqueGamePtr<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDeleter>(){
		this->reset(_ptr);
	}
};

template<typename T>
struct UniqueGamePtrB : public std::unique_ptr<T, GameDTORCaller>{

	COMPILETIMEEVAL UniqueGamePtrB<T>() noexcept : std::unique_ptr<T, GameDTORCaller>()
	{ }

	COMPILETIMEEVAL UniqueGamePtrB<T>(T* _ptr) noexcept : std::unique_ptr<T, GameDTORCaller>() {
		this->reset(_ptr);
	}
};

//template <typename T>
//using UniqueGamePtrB = std::unique_ptr<T, GameDTORCaller>;

//template <typename T>
//using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;
