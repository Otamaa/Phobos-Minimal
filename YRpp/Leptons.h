#pragma once
#include <Unsorted.h>

struct Leptons
{
	constexpr Leptons() noexcept = default;
	//explicit Leptons(int value) noexcept : value(value) {}
	explicit constexpr Leptons(const int value) noexcept : value(value) { }
	explicit constexpr Leptons(double velue) noexcept : value(int(velue * Unsorted::d_LeptonsPerCell)) { }

	constexpr Leptons(const Leptons&) noexcept = default;
	constexpr Leptons(Leptons&&) noexcept =  default;
	constexpr Leptons&operator=(const Leptons& other) noexcept = default;

	constexpr inline operator int() const
	{ return this->value; }

	constexpr inline unsigned long ToLong() const
	{ return static_cast<std::make_unsigned<long>::type> (this->value); }

	constexpr inline double ToDouble() const
	{ return static_cast<double>(this->value / Unsorted::d_LeptonsPerCell); }

	constexpr inline int ToCell() const
	{ return this->value / Unsorted::LeptonsPerCell; }

	int value { 0 };
};

static_assert(sizeof(Leptons) == sizeof(int), "Invalid Size !");