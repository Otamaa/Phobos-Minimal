#pragma once

#include <memory>
#include <Memory.h>

template<typename T>
struct UniqueDLLPtr : public std::unique_ptr<T, DLLDeleter> {

	COMPILETIMEEVAL UniqueDLLPtr<T>() noexcept : std::unique_ptr<T, DLLDeleter>()
	{ }

	COMPILETIMEEVAL UniqueDLLPtr<T>(T* _ptr) noexcept : std::unique_ptr<T, DLLDeleter>() {
		this->reset(_ptr);
	}
};