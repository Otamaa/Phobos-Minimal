#pragma once

#include <YRMath.h>

namespace PhobosMathCache
{
	//TODO : float
	//TODO : int
	//using Computer = double(*)();

	//struct Entry
	//{
	//	const char* name;     // for debugging
	//	Computer    compute;
	//	double      cached;
	//};

	//// Define your constants as a static table of (name, function)
	//inline Entry Entries[] = {
	//	{ "SlopeShallow_InvLen", []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "SlopeMedium_InvLen",  []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "SlopeSteep_InvLen",   []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "Deg45_Sin",           []() { return sin(45.0 * Math::GAME_PI / 180.0); }, 0.0 },
	//};

	//enum Index : size_t
	//{
	//	SlopeShallow_InvLen,
	//	SlopeMedium_InvLen,
	//	SlopeSteep_InvLen,
	//	Deg45_Sin,
	//};

	//inline void Recalculate()
	//{
	//	for (auto& e : Entries) e.cached = e.compute();
	//}

	//[[nodiscard]] inline double Get(Index i) noexcept
	//{
	//	return Entries[i].cached;
	//}
}