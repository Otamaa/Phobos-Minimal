#pragma once

#include <type_traits>
#include <concepts>

template<typename T>
concept direct_comparable = std::is_pointer_v<T> || std::is_integral_v<T> || std::equality_comparable<T>;
