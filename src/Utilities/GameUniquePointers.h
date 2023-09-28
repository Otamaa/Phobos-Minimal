#pragma once

#include <Memory.h>

template <typename T>
using UniqueGamePtrB = std::unique_ptr<T, GameDTORCaller>;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;
