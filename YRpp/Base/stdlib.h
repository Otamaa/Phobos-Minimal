#pragma once
#include "Always.h"
#ifndef __cpp_lib_bit_cast
#include <type_traits>
#endif

#if __cplusplus == 201402L
#include <algorithm>
#endif

namespace std
{
#ifndef __cpp_lib_bit_cast
	template <class To, class From>
	std::enable_if_t<
		sizeof(To) == sizeof(From) &&
		std::is_trivially_copyable_v<From> &&
		std::is_trivially_copyable_v<To>,
		To>
		// constexpr support needs compiler magic
		bit_cast(const From& src) noexcept
	{
		static_assert(std::is_trivially_constructible_v<To>,
			"This implementation additionally requires destination type to be trivially constructible");

		To dst;
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}
#endif

#if __cplusplus == 201402L
	template<class T, class Compare>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
	{
		return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
	}

	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi)
	{
		return clamp(v, lo, hi, std::less<T>());
	}
#endif
}

