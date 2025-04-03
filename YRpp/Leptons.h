#pragma once
#include <Unsorted.h>

struct Leptons
{
	COMPILETIMEEVAL Leptons() noexcept = default;
	//explicit Leptons(int value) noexcept : value(value) {}
	explicit COMPILETIMEEVAL Leptons(const int value) noexcept : value(value) { }
	explicit COMPILETIMEEVAL Leptons(double velue) noexcept : value(int(velue * Unsorted::d_LeptonsPerCell)) { }

	COMPILETIMEEVAL Leptons(const Leptons&) noexcept = default;
	COMPILETIMEEVAL Leptons(Leptons&&) noexcept =  default;
	COMPILETIMEEVAL Leptons&operator=(const Leptons& other) noexcept = default;

	COMPILETIMEEVAL OPTIONALINLINE operator int() const
	{ return this->value; }

	COMPILETIMEEVAL OPTIONALINLINE unsigned long ToLong() const
	{ return static_cast<std::make_unsigned<long>::type> (this->value); }

	COMPILETIMEEVAL OPTIONALINLINE double ToDouble() const
	{ return static_cast<double>(this->value / Unsorted::d_LeptonsPerCell); }

	COMPILETIMEEVAL OPTIONALINLINE int ToCell() const
	{ return this->value / Unsorted::LeptonsPerCell; }

	int value { 0 };
};

static_assert(sizeof(Leptons) == sizeof(int), "Invalid Size !");
