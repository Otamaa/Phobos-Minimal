#pragma once

#include <cstddef>
#include <type_traits>

// EnumArray<T, Enum, Count>
// Fixed-size array indexed directly by a scoped enum.
// Enum must have an explicit integral underlying type.
// Count defaults to the enum's "Count" sentinel if present.
//
// Usage:
//   EnumArray<AnimClass*, BuildingAnimSlot, BuildingAnimSlot::Count> Anims;
//   Anims[BuildingAnimSlot::SuperThree] = ...;

template<typename T, typename TEnum, size_t TCount>
class EnumArray
{
	static_assert(std::is_enum_v<TEnum>, "TEnum must be an enum type");

	static constexpr std::size_t N = static_cast<std::size_t>(TCount);
	static_assert(N > 0, "EnumArray size must be > 0");

public:
	using value_type = T;
	using enum_type = TEnum;
	using index_type = std::underlying_type_t<TEnum>;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;
	using iterator = T*;
	using const_iterator = const T*;

	// ── Element access ───────────────────────────────────────────────────────

	[[nodiscard]] constexpr reference operator[](TEnum slot) noexcept
	{
		return Data[static_cast<index_type>(slot)];
	}

	[[nodiscard]] constexpr const_reference operator[](TEnum slot) const noexcept
	{
		return Data[static_cast<index_type>(slot)];
	}

	// Raw integer access — kept for interop with vanilla code paths.
	[[nodiscard]] constexpr reference operator[](index_type idx) noexcept
	{
		return Data[idx];
	}

	[[nodiscard]] constexpr const_reference operator[](index_type idx) const noexcept
	{
		return Data[idx];
	}

	// ── Iterators ────────────────────────────────────────────────────────────

	[[nodiscard]] constexpr iterator       begin()  noexcept { return Data; }
	[[nodiscard]] constexpr iterator       end()    noexcept { return Data + N; }
	[[nodiscard]] constexpr const_iterator begin()  const noexcept { return Data; }
	[[nodiscard]] constexpr const_iterator end()    const noexcept { return Data + N; }
	[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return Data; }
	[[nodiscard]] constexpr const_iterator cend()   const noexcept { return Data + N; }

	// ── Capacity ─────────────────────────────────────────────────────────────

	[[nodiscard]] static constexpr std::size_t size()  noexcept { return N; }
	[[nodiscard]] static constexpr bool        empty() noexcept { return N == 0; }

	// ── Raw data — for vanilla memcpy / legacy pointer arithmetic ────────────

	[[nodiscard]] constexpr pointer       data()       noexcept { return Data; }
	[[nodiscard]] constexpr const_pointer data() const noexcept { return Data; }

public :

	// ── Storage — public to preserve aggregate initialisation & POD layout ───
	T Data[N];
};

// ── Deduction guide (C++17) ──────────────────────────────────────────────────
// Lets you write: EnumArray arr { BuildingAnimSlot::Count, (AnimClass*)nullptr };
// (rarely needed; explicit template args are clearer)