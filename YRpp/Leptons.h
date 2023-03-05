#pragma once
#include <Unsorted.h>

struct Leptons
{
	Leptons() = default;
	//explicit Leptons(int value) noexcept : value(value) {}
	explicit Leptons(const int value) noexcept : value(value) { }
	explicit Leptons(double velue) noexcept : value(Game::F2I(velue * Unsorted::d_LeptonsPerCell)) { }

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

struct CompileTimeLeptons
{
	//explicit constexpr CompileTimeLeptons(int value) noexcept : value(value) { }
	explicit constexpr CompileTimeLeptons(const int value) noexcept : value(value) { }
	explicit constexpr CompileTimeLeptons(double velue) noexcept : value(static_cast<int>(velue* Unsorted::d_LeptonsPerCell)) { }

	operator Leptons() const
	{ return Leptons(this->value); }

	int value { 0 };
};

static_assert(sizeof(Leptons) == sizeof(int), "Invalid Size !");