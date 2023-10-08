#pragma once
#include <Unsorted.h>

struct Leptons
{
	Leptons() = default;
	//explicit Leptons(int value) noexcept : value(value) {}
	explicit Leptons(const int value) noexcept : value(value) { }
	explicit Leptons(double velue) noexcept : value(int(velue * Unsorted::d_LeptonsPerCell)) { }

	Leptons(const Leptons&) = default;
	Leptons(Leptons&&) = default;
	Leptons&operator=(const Leptons& other) = default;

	operator int() const
	{ return this->value; }

	inline unsigned long ToLong() const
	{ return static_cast<std::make_unsigned<long>::type> (this->value); }

	inline double ToDouble() const
	{ return static_cast<double>(this->value / Unsorted::d_LeptonsPerCell); }

	inline int ToCell() const
	{ return this->value / Unsorted::LeptonsPerCell; }

	int value { 0 };
};

static_assert(sizeof(Leptons) == sizeof(int), "Invalid Size !");