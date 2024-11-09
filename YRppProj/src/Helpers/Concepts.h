#pragma once

#include <type_traits>
#include <concepts>

template<typename T>
concept direct_comparable = std::is_pointer<T>::value || std::is_integral<T>::value || std::equality_comparable<T>;
